// client.c
/* Filename:
 *      client.c
 *
 * Description:
 *      This is the client for the game.
 *
 * Compile Instructions:
 *      `gcc -o client client.c`
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>

// Function to handle errors
void die_with_error(char *error_msg) {
    perror(error_msg);
    exit(-1);
}

// Function to display health bar
void display_health_bar(const char* player_label, int hp) {
    int holder = hp / 10;
    char healthbar[11] = {0}; // Initialize with null characters
    for(int i = 0; i < holder; i++) {
        healthbar[i] = '=';
    }
    printf("%s HP: ", player_label);
    for(int i = 0; i < holder; i++) {
        printf("%c", healthbar[i]);
    }
    printf(" (%d)\n", hp);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <Server IP> <Server Port> <Client ID>\n", argv[0]);
        return -1;
    }

    int client_socket;
    struct sockaddr_in server_addr;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
        die_with_error("Error: socket() failed");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        die_with_error("Error: connect() failed");

    printf("Connected to the game server. Welcome! Get Ready to play RPS Arena!\n");
    printf("\n");

    // Main game loop
    int my_hp, opponent_hp;
    char choice;
    char round_winner[256];
    while (1) {
        // Input your move
        printf("Enter your choice (Rock [r], Paper [p], Scissors [s]): ");
        scanf(" %c", &choice);
        while (choice != 'r' && choice != 'p' && choice != 's') {
            printf("Invalid input. Please enter 'r', 'p', or 's': ");
            scanf(" %c", &choice);
        }

        // Send your move
        if (send(client_socket, &choice, sizeof(choice), 0) < 0)
            die_with_error("Error: send() failed");

        // Receive round winner message
        if (recv(client_socket, round_winner, sizeof(round_winner), 0) < 0)
            die_with_error("Error: recv() failed");
        printf("%s\n", round_winner);  // Add a blank line after the round winner message

        // Receive your HP
        if (recv(client_socket, &my_hp, sizeof(my_hp), 0) < 0)
            die_with_error("Error: recv() failed");
            
        // Display health bar for Player 1
        display_health_bar("Player 1", my_hp);
        printf("\n");

        // Receive opponent's HP
        if (recv(client_socket, &opponent_hp, sizeof(opponent_hp), 0) < 0)
            die_with_error("Error: recv() failed");
        
        // Display health bar for Player 2
        display_health_bar("Player 2", opponent_hp);
        printf("\n");

        // Check if the game is over
        if (my_hp <= 0) {
            printf("You lose!\n");
            break;
        }
        if (opponent_hp <= 0) {
            printf("You win!\n");
            break;
        }
    }

    // Close socket
    close(client_socket);
    return 0;
}

