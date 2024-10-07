#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
  int sockfd;
  struct sockaddr_in server_addr;
  char buffer[BUFFER_SIZE];
  struct pollfd fds[2];

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

  // Set up the pollfd structure to monitor stdin and the socket
  fds[0].fd = STDIN_FILENO; // Monitor user input (stdin)
  fds[0].events = POLLIN;
  fds[1].fd = sockfd;       // Monitor server messages (socket)
  fds[1].events = POLLIN;

  while (1) {
    int poll_count = poll(fds, 2, -1);  // Wait indefinitely for an event

    if (poll_count < 0) {
      perror("Poll failed");
      exit(EXIT_FAILURE);
    }

    // Check for input from the server
    if (fds[1].revents & POLLIN) {
      int valread = read(sockfd, buffer, BUFFER_SIZE);
      if (valread == 0) {
          printf("Server disconnected.\n");
          close(sockfd);
          break;
      }
      buffer[valread] = '\0';
      printf("Message from server: %s\n", buffer);
    }

    // Check for user input (stdin)
    if (fds[0].revents & POLLIN) {
      fgets(buffer, BUFFER_SIZE, stdin);
      send(sockfd, buffer, strlen(buffer), 0);
    }
  }

  return 0;
}
