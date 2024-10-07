#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

int main() {
  int server_fd, new_socket, client_sockets[MAX_CLIENTS], max_clients = MAX_CLIENTS, addrlen, activity, i, valread, sd;
  struct sockaddr_in address;
  char buffer[BUFFER_SIZE];

  // Set of socket descriptors
  fd_set readfds;

  // Initialize all client_sockets[] to 0
  for (i = 0; i < max_clients; i++) {
    client_sockets[i] = 0;
  }

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

  while (1) {
    // Clear the socket set
    FD_ZERO(&readfds);

    // Add server socket to set
    FD_SET(server_fd, &readfds);
    int max_sd = server_fd;

    // Add client sockets to set
    for (i = 0; i < max_clients; i++) {
      sd = client_sockets[i];
      if (sd > 0) FD_SET(sd, &readfds);
      if (sd > max_sd) max_sd = sd;
    }

    // Wait for activity on one of the sockets
    activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

    // Incoming connection
    if (FD_ISSET(server_fd, &readfds)) {
      if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
      }
      printf("New connection, socket fd is %d, ip is: %s, port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

      // Add new socket to array
      for (i = 0; i < max_clients; i++) {
        if (client_sockets[i] == 0) {
          client_sockets[i] = new_socket;
          printf("Adding to list of sockets as %d\n", i);
          break;
        }
      }
    }

    // Handle I/O operations on clients
    for (i = 0; i < max_clients; i++) {
      sd = client_sockets[i];

      if (FD_ISSET(sd, &readfds)) {
        // Check if it was for closing, and read the incoming message
        if ((valread = read(sd, buffer, BUFFER_SIZE)) == 0) {
          // Someone disconnected, get details
          getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
          printf("Client disconnected, ip %s, port %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

          // Close the socket and mark as 0
          close(sd);
          client_sockets[i] = 0;

          // Inform other clients that a user has disconnected
          char disconnect_msg[BUFFER_SIZE];
          snprintf(disconnect_msg, sizeof(disconnect_msg), "Client at %s has disconnected.\n", inet_ntoa(address.sin_addr));
          for (int j = 0; j < max_clients; j++) {
            if (client_sockets[j] > 0) {
              send(client_sockets[j], disconnect_msg, strlen(disconnect_msg), 0);
            }
          }
        } else {
          // Broadcast message to all other clients
          buffer[valread] = '\0';
          for (int j = 0; j < max_clients; j++) {
            if (client_sockets[j] > 0 && client_sockets[j] != sd) {
              send(client_sockets[j], buffer, strlen(buffer), 0);
            }
          }
        }
      }
    }
  }

  return 0;
}
