// server.c
/* Filename:
 *      server.c
 *
 * Description:
 *      This is the server to handle the game for two players.
 *
 * Compile Instructions:
 *      `gcc -o server server.c`
 *
 * Author:
 *
 *      Charies Ann Hao
 *      Christian Del Rosario
 *      Gabriel Señar
 *      James Andrei Aguilar
 *      Miguel Armand Sta Ana
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_HP 100
#define BASE_DAMAGE 10
#define DOUBLE_DAMAGE 20
#define STREAK_THRESHOLD 3

// Function to handle errors
void die_with_error(char *error_msg) {
    perror(error_msg);
    exit(-1);
}

// Function to handle a game round
int game_round(char p1_choice, char p2_choice) {
    // Returns 0 for draw, 1 for p1 wins, 2 for p2 wins
    if (p1_choice == p2_choice) return 0;
    if ((p1_choice == 'r' && p2_choice == 's') ||
        (p1_choice == 's' && p2_choice == 'p') ||
        (p1_choice == 'p' && p2_choice == 'r')) return 1;
    return 2;
}

int main(int argc, char *argv[]) {
    int server_sock, client_sock1, client_sock2, port_no;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_size = sizeof(client_addr);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s port_no\n", argv[0]);
        exit(1);
    }

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
        die_with_error("Error: socket() Failed.");

    memset(&server_addr, 0, sizeof(server_addr));
    port_no = atoi(argv[1]);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_no);

    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        die_with_error("Error: bind() Failed.");

    listen(server_sock, 2);
    printf("Server listening on port %d ...\n", port_no);

    // Accept connections from two clients
    client_sock1 = accept(server_sock, (struct sockaddr *) &client_addr, &client_size);
    if (client_sock1 < 0)
        die_with_error("Error: accept() Failed for Player 1.");

    printf("Player 1 connected: %s\n", inet_ntoa(client_addr.sin_addr));

    client_sock2 = accept(server_sock, (struct sockaddr *) &client_addr, &client_size);
    if (client_sock2 < 0)
        die_with_error("Error: accept() Failed for Player 2.");

    printf("Player 2 connected: %s\n", inet_ntoa(client_addr.sin_addr));

    // Initialize game variables
    int p1_hp = MAX_HP, p2_hp = MAX_HP;
    int p1_streak = 0, p2_streak = 0;
    int p1_damage = BASE_DAMAGE, p2_damage = BASE_DAMAGE;
    char p1_choice, p2_choice;
    int result;
    char round_winner[256];

    // Game loop
    while (p1_hp > 0 && p2_hp > 0) {
        // Receive choice from Player 1
        if (recv(client_sock1, &p1_choice, 1, 0) < 0)
            die_with_error("Error: recv() from Client 1 Failed.");
        printf("Received from Player 1: %c\n", p1_choice);

        // Receive choice from Player 2
        if (recv(client_sock2, &p2_choice, 1, 0) < 0)
            die_with_error("Error: recv() from Client 2 Failed.");
        printf("Received from Player 2: %c\n", p2_choice);

        // Game logic
        result = game_round(p1_choice, p2_choice);
        
        // Determine round winner and update health and streaks
        if (result == 1) { // Player 1 wins
            p2_hp -= p1_damage;
            if (p2_hp < 0) p2_hp = 0; // Ensure HP does not go negative
            p1_streak++;
            p2_streak = 0;
            strcpy(round_winner, "Player 1 wins this round!\n");
            if (p1_streak >= STREAK_THRESHOLD) {
                p1_damage = DOUBLE_DAMAGE;
                strcat(round_winner, "Double damage activated for Player 1!\n");
            }
        } else if (result == 2) { // Player 2 wins
            p1_hp -= p2_damage;
            if (p1_hp < 0) p1_hp = 0; // Ensure HP does not go negative
            p2_streak++;
            p1_streak = 0;
            strcpy(round_winner, "Player 2 wins this round!\n");
            if (p2_streak >= STREAK_THRESHOLD) {
                p2_damage = DOUBLE_DAMAGE;
                strcat(round_winner, "Double damage activated for Player 2!\n");
            }
        } else { // Draw
            p1_streak = 0;
            p2_streak = 0;
            p1_damage = BASE_DAMAGE;
            p2_damage = BASE_DAMAGE;
            strcpy(round_winner, "This round is a draw!\n");
        }

        // Append streak information to the round winner message
        char streak_info[256];
        sprintf(streak_info, "Player 1 Streak: %d, Player 2 Streak: %d\n", p1_streak, p2_streak);
        strcat(round_winner, streak_info);

        // Send round winner to clients
        if (send(client_sock1, round_winner, sizeof(round_winner), 0) < 0)
            die_with_error("Error: send() to Client 1 Failed.");
        if (send(client_sock2, round_winner, sizeof(round_winner), 0) < 0)
            die_with_error("Error: send() to Client 2 Failed.");

        // Send updated health to clients
        if (send(client_sock1, &p1_hp, sizeof(p1_hp), 0) < 0)
            die_with_error("Error: send() to Client 1 Failed.");
        if (send(client_sock1, &p2_hp, sizeof(p2_hp), 0) < 0)
            die_with_error("Error: send() to Client 1 Failed.");
        
        if (send(client_sock2, &p2_hp, sizeof(p2_hp), 0) < 0)
            die_with_error("Error: send() to Client 2 Failed.");
        if (send(client_sock2, &p1_hp, sizeof(p1_hp), 0) < 0)
            die_with_error("Error: send() to Client 2 Failed.");

        printf("Sent to Player 1: Your HP: %d, Opponent's HP: %d\n", p1_hp, p2_hp);
        printf("Sent to Player 2: Your HP: %d, Opponent's HP: %d\n", p2_hp, p1_hp);

        // Check if the game should end
        if (p1_hp <= 0 || p2_hp <= 0) {
            char end_msg[256];
            if (p1_hp <= 0 && p2_hp <= 0)
                sprintf(end_msg, "Draw Game!\n");
            else if (p1_hp <= 0)
                sprintf(end_msg, "Game over, Player 2 Wins!\n");
            else
                sprintf(end_msg, "Game over, Player 1 Wins!\n");
            
            send(client_sock1, end_msg, strlen(end_msg), 0);
            send(client_sock2, end_msg, strlen(end_msg), 0);
            break;
        }
    }

    // Close client sockets
    close(client_sock1);
    close(client_sock2);
    close(server_sock);

    return 0;
}

