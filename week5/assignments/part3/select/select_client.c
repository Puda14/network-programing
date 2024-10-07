#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
  int sockfd;
  struct sockaddr_in server_addr;
  char buffer[BUFFER_SIZE];
  fd_set read_fds;

  // Create client socket
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Socket creation error");
    exit(EXIT_FAILURE);
  }

  // Set up the server address
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Connect to server
  if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    perror("Connection failed");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  printf("Connected to server. You can start sending messages...\n");

  while (1) {
    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds);  // Monitor user input
    FD_SET(sockfd, &read_fds);        // Monitor server messages

    // Wait for activity
    int activity = select(sockfd + 1, &read_fds, NULL, NULL, NULL);

    // Check for input from the server
    if (FD_ISSET(sockfd, &read_fds)) {
      int valread = read(sockfd, buffer, BUFFER_SIZE);
      if (valread == 0) {
        printf("Server disconnected.\n");
        close(sockfd);
        break;
      }
      buffer[valread] = '\0';
      printf("Message from server: %s\n", buffer);
    }

    // Check for user input
    if (FD_ISSET(STDIN_FILENO, &read_fds)) {
      fgets(buffer, BUFFER_SIZE, stdin);
      send(sockfd, buffer, strlen(buffer), 0);
    }
  }

  return 0;
}
