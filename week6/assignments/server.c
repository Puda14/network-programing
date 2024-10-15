#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>

#define PORT 8080
#define MAXLINE 1024
#define TIMEOUT 5
#define MAX_CLIENTS 10

// Improved XOR cipher with more security (example: multiple keys)
void improved_xor_cipher(char *data, const char *key, int key_len) {
    for (int i = 0; data[i] != '\0'; i++) {
        data[i] ^= key[i % key_len];
    }
}

int main() {
    int sockfd;
    char buffer[MAXLINE];
    char *messages[] = {
        "Hello from server",
        "Good day!",
        "Welcome to the server",
        "You are connected",
        "Random server message"
    };
    int num_messages = sizeof(messages) / sizeof(messages[0]);
    struct sockaddr_in servaddr, cliaddr;
    int client_sockets[MAX_CLIENTS];
    fd_set readfds;
    struct timeval tv;
    socklen_t len;
    int max_sd, activity, n;

    // Initialize random seed for rand()
    srand(time(NULL));

    // Initialize all client sockets to 0
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
    }

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Bind socket to IP and port
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server is running...\n");

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        max_sd = sockfd;

        // Add client sockets to the set
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] > 0) {
                FD_SET(client_sockets[i], &readfds);
            }
            if (client_sockets[i] > max_sd) {
                max_sd = client_sockets[i];
            }
        }

        // Set timeout values
        tv.tv_sec = TIMEOUT;
        tv.tv_usec = 0;

        // Wait for activity on one of the sockets
        activity = select(max_sd + 1, &readfds, NULL, NULL, &tv);

        if (activity == -1) {
            perror("select error");
            close(sockfd);
            exit(EXIT_FAILURE);
        } else if (activity == 0) {
            printf("Timeout: No activity from clients in the last %d seconds.\n", TIMEOUT);
            continue;
        }

        // If there's activity on the server socket, handle new client
        if (FD_ISSET(sockfd, &readfds)) {
            len = sizeof(cliaddr);
            n = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
            buffer[n] = '\0';

            printf("Received message from client: %s\n", buffer);

            // Add the client to the list of active clients
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = sockfd;
                    break;
                }
            }

            // Select a random message
            int random_index = rand() % num_messages;
            char *selected_message = messages[random_index];

            // Encrypt the selected message with an improved XOR cipher
            char key[] = "ComplexKey";
            int key_len = strlen(key);
            char msg_to_send[MAXLINE];
            strncpy(msg_to_send, selected_message, MAXLINE - 1);
            msg_to_send[MAXLINE - 1] = '\0';

            improved_xor_cipher(msg_to_send, key, key_len);

            sendto(sockfd, msg_to_send, strlen(msg_to_send), 0, (struct sockaddr *)&cliaddr, len);
            printf("Encrypted message sent to client: %s\n", selected_message);
        }
    }

    close(sockfd);
    return 0;
}
