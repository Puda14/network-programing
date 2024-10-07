#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

// Function to broadcast a message to all clients except the sender
void broadcast_message(int sender_fd, struct pollfd *fds, int num_clients, char *message) {
for (int i = 1; i < num_clients; i++) { // Start at 1 to skip server socket
    int client_fd = fds[i].fd;
    if (client_fd != sender_fd && client_fd > 0) {
        send(client_fd, message, strlen(message), 0);
    }
  }
}

int main() {
  int server_fd, new_socket, addrlen, valread, i;
  struct sockaddr_in address;
  char buffer[BUFFER_SIZE];
  struct pollfd fds[MAX_CLIENTS];

  // Create server socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("Socket failed");
    exit(EXIT_FAILURE);
  }

  // Configure server address
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // Bind socket to the address and port
  if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
    perror("Bind failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  // Listen for incoming connections
  if (listen(server_fd, 3) < 0) {
    perror("Listen failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  addrlen = sizeof(address);
  printf("Listening on port %d...\n", PORT);

  // Initialize the pollfd structure
  fds[0].fd = server_fd;     // The server's listening socket
  fds[0].events = POLLIN;    // We are interested in read events (POLLIN)
  for (i = 1; i < MAX_CLIENTS; i++) {
      fds[i].fd = -1;        // Initialize all client fds to -1 (indicating empty slots)
  }

  while (1) {
    int poll_count = poll(fds, MAX_CLIENTS, -1);  // Wait indefinitely until an event occurs

    if (poll_count < 0) {
      perror("Poll failed");
      exit(EXIT_FAILURE);
    }

    // Check for new connection on server socket
    if (fds[0].revents & POLLIN) {
      new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
      if (new_socket < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
      }
      printf("New connection, socket fd is %d, ip is: %s, port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

      // Add new client socket to the pollfd array
      for (i = 1; i < MAX_CLIENTS; i++) {
        if (fds[i].fd == -1) {
          fds[i].fd = new_socket;
          fds[i].events = POLLIN;  // Monitor this socket for read events
          break;
        }
      }

      if (i == MAX_CLIENTS) {
        printf("Too many clients connected. Connection rejected.\n");
        close(new_socket);  // Too many clients connected, reject the new one
      }
    }

    // Check for data from clients
    for (i = 1; i < MAX_CLIENTS; i++) {
      if (fds[i].revents & POLLIN) {
        // Read incoming message
        valread = read(fds[i].fd, buffer, BUFFER_SIZE);
        if (valread == 0) {
          // Client disconnected
          getpeername(fds[i].fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
          printf("Client disconnected, ip %s, port %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
          close(fds[i].fd);
          fds[i].fd = -1;  // Mark the slot as available
        } else {
          // Broadcast message to all other clients
          buffer[valread] = '\0';
          broadcast_message(fds[i].fd, fds, MAX_CLIENTS, buffer);
        }
      }
    }
  }

  return 0;
}
