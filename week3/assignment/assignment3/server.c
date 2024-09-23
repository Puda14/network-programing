#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXLINE 1024
#define PORT 8080

// Custom sock_ntop function to convert a socket address into a string (IP and port)
char *sock_ntop(const struct sockaddr *sa, socklen_t salen) {
    static char str[128];
    char portstr[8];

    if (sa->sa_family == AF_INET) {
        struct sockaddr_in *sin = (struct sockaddr_in *)sa;
        if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
            return NULL;
        snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port));
        strcat(str, portstr);
        return str;
    }

    return NULL;
}

// Function to read a line (up to \n) from a socket descriptor
ssize_t readline(int fd, void *vptr, size_t maxlen) {
    ssize_t n, rc;
    char c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
        if ((rc = read(fd, &c, 1)) == 1) {
            *ptr++ = c;
            if (c == '\n') {
                break;  // Stop at newline
            }
        } else if (rc == 0) {
            if (n == 1) {
                return 0; // No data read
            } else {
                break;  // Some data was read
            }
        } else {
            return -1; // Error in read
        }
    }
    *ptr = 0;
    return n;
}

int main() {
    int listenfd, connfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[MAXLINE];
    socklen_t clilen;
    ssize_t n;
    char client_addr_str[128];

    // Create a listening socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Bind the socket to the address and port
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(listenfd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept incoming connection from client
    clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
    if (connfd < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    // Convert client's IP address from binary to string using sock_ntop
    printf("Client connected from %s\n", sock_ntop((struct sockaddr *)&cliaddr, clilen));

    // Read the message from the client
    while ((n = readline(connfd, buffer, MAXLINE)) > 0) {
        buffer[n] = '\0';
        printf("Received message: %s", buffer);

        // Send the received message back to the client
        if (write(connfd, buffer, n) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
        }
    }

    if (n < 0) {
        perror("Readline failed");
        exit(EXIT_FAILURE);
    }

    close(connfd);
    close(listenfd);
    return 0;
}
