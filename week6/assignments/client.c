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
    char *response = "Response from client";
    struct sockaddr_in servaddr, recv_servaddr;
    char *server_ip = "127.0.0.1";  // Server IP address

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&recv_servaddr, 0, sizeof(recv_servaddr));

    // Fill server information (IP and port)
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, server_ip, &servaddr.sin_addr);

    // Send message to server
    sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    printf("Message sent to server.\n");

    // Use select() to handle timeout
    fd_set readfds;
    struct timeval tv;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;

    // Wait for server response
    int activity = select(sockfd + 1, &readfds, NULL, NULL, &tv);
    if (activity == -1) {
        perror("select error");
        close(sockfd);
        exit(EXIT_FAILURE);
    } else if (activity == 0) {
        printf("Timeout: No response from server.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Receive encrypted message from server
    socklen_t len = sizeof(recv_servaddr);
    int n = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *)&recv_servaddr, &len);
    buffer[n] = '\0';  // Ensure string termination

    // XOR decryption
    char key = 'K';
    xor_cipher(buffer, key);
    printf("Decrypted message from server: %s\n", buffer);

    // Verify server address using memcmp()
    if (memcmp(&servaddr, &recv_servaddr, sizeof(struct sockaddr_in)) == 0) {
        printf("Verified: Message from the correct server.\n");
    } else {
        printf("Warning: Message from an unknown server.\n");
    }

    close(sockfd);
    return 0;
}
