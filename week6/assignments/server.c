#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define MAXLINE 1024
#define TIMEOUT 5  // 5 seconds timeout for select()

// XOR cipher function for encryption/decryption
void xor_cipher(char *data, char key) {
    for (int i = 0; data[i] != '\0'; i++) {
        data[i] ^= key;
    }
}

int main() {
    int sockfd;
    char buffer[MAXLINE];
    char *messages[] = {"Hello Client 1", "Hello Client 2", "Hello Client 3"};
    struct sockaddr_in servaddr, cliaddr;

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

    fd_set readfds;
    struct timeval tv;
    socklen_t len;
    int n;

    while (1) {
        len = sizeof(cliaddr);

        // Use select() to wait for data with timeout
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        tv.tv_sec = TIMEOUT;
        tv.tv_usec = 0;

        int activity = select(sockfd + 1, &readfds, NULL, NULL, &tv);

        if (activity == -1) {
            perror("select error");
            close(sockfd);
            exit(EXIT_FAILURE);
        } else if (activity == 0) {
            printf("Timeout: No activity from clients in the last %d seconds.\n", TIMEOUT);
            continue;
        }

        // Receive message from client
        n = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0';  // Ensure string termination
        printf("Received message from client: %s\n", buffer);

        // Encrypt a random message with XOR and send it to client
        char key = 'K';
        int client_id = rand() % 3; // Randomly select a message for the client

        // Create a copy of the selected message to perform XOR safely
        char msg_to_send[MAXLINE];
        strncpy(msg_to_send, messages[client_id], MAXLINE - 1);
        msg_to_send[MAXLINE - 1] = '\0'; // Ensure null-termination

        xor_cipher(msg_to_send, key);  // Encrypt message

        sendto(sockfd, msg_to_send, strlen(msg_to_send), 0, (struct sockaddr *)&cliaddr, len);
        printf("Encrypted message sent to client.\n");
    }

    close(sockfd);
    return 0;
}
