#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXLINE 1024
#define PORT 8080

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char sendline[MAXLINE], recvline[MAXLINE];
    ssize_t n;
    char server_ip[INET_ADDRSTRLEN];

    // Ask the user to input the server's IP address
    printf("Enter server IP address: ");
    scanf("%s", server_ip);
    getchar();  // Consume the newline left by scanf

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);

    // Convert the entered server IP address to binary format and set it
    if (inet_pton(AF_INET, server_ip, &servaddr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Send a message to the server
    printf("Enter message: ");
    fgets(sendline, MAXLINE, stdin);  // Read message from user
    write(sockfd, sendline, strlen(sendline));  // Send message to server

    // Read the server's response
    if ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = '\0';  // Null-terminate the received string
        printf("Server response: %s", recvline);
    } else {
        perror("Read failed");
    }

    close(sockfd);
    return 0;
}
