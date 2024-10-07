#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

typedef struct {
  int fd;                  // File descriptor for the socket
  char username[100];       // Username for the client
} Client;

volatile sig_atomic_t stop_server = 0;

void handle_signal(int sig) {
  stop_server = 1;
  printf("\033[1;31mCaught signal %d\033[0m\n", sig);
}

void setup_signal_handling() {
  struct sigaction sa;
  sa.sa_handler = handle_signal;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);
}

void broadcast_message(Client *clients, int num_clients, char *message, int sender_fd) {
  for (int i = 0; i < num_clients; i++) {
    if (clients[i].fd != sender_fd && clients[i].fd > 0) {
      send(clients[i].fd, message, strlen(message), 0);
    }
  }
}

int main() {
  int server_fd, new_socket, addrlen, i, valread;
  struct sockaddr_in address;
  char buffer[BUFFER_SIZE];
  Client clients[MAX_CLIENTS];
  fd_set readfds;

  setup_signal_handling();

  // Initialize all client sockets and usernames
  for (i = 0; i < MAX_CLIENTS; i++) {
    clients[i].fd = 0;
    strcpy(clients[i].username, "");
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
  printf("\033[1;32mListening on port %d...\033[0m\n", PORT);

  while (!stop_server) {
    FD_ZERO(&readfds);
    FD_SET(server_fd, &readfds);
    int max_sd = server_fd;

    // Add client sockets to the read set
    for (i = 0; i < MAX_CLIENTS; i++) {
      if (clients[i].fd > 0) {
        FD_SET(clients[i].fd, &readfds);
      }
      if (clients[i].fd > max_sd) {
        max_sd = clients[i].fd;
      }
    }

    int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

    if (activity < 0 && stop_server) {
      break;
    }

    // Handle new connection
    if (FD_ISSET(server_fd, &readfds)) {
      new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
      if (new_socket < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
      }

      // Get client IP and port
      char client_ip[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &address.sin_addr, client_ip, INET_ADDRSTRLEN);
      int client_port = ntohs(address.sin_port);

      // Add new client to the array
      for (i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd == 0) {
          clients[i].fd = new_socket;

          // Read username from client
          valread = read(new_socket, clients[i].username, sizeof(clients[i].username));
          clients[i].username[valread] = '\0';

          // Print the client information on the server side
          printf("\033[1;35mNew client joined: %s (IP: %s, Port: %d)\033[0m\n", clients[i].username, client_ip, client_port);

          // Notify other clients
          char join_message[BUFFER_SIZE];
          snprintf(join_message, sizeof(join_message), "\033[1;35m%s has joined the chat.\033[0m\n", clients[i].username);
          broadcast_message(clients, MAX_CLIENTS, join_message, new_socket);
          break;
        }
      }

      if (i == MAX_CLIENTS) {
        printf("\033[1;33mMax clients reached. Rejecting connection.\033[0m\n");
        close(new_socket);
      }
    }

    // Check for messages from clients
    for (i = 0; i < MAX_CLIENTS; i++) {
      int sd = clients[i].fd;
      if (FD_ISSET(sd, &readfds)) {
        valread = read(sd, buffer, BUFFER_SIZE);
        if (valread == 0) {
          // Client disconnected
          printf("\033[1;31mClient %s disconnected\033[0m\n", clients[i].username);
          close(sd);
          clients[i].fd = 0;

          // Notify other clients
          char disconnect_message[BUFFER_SIZE];
          snprintf(disconnect_message, sizeof(disconnect_message), "\033[1;31m%s has left the chat.\033[0m\n", clients[i].username);
          broadcast_message(clients, MAX_CLIENTS, disconnect_message, 0);
        } else {
          // Broadcast message with username
          buffer[valread] = '\0';
          char message_with_username[BUFFER_SIZE];
          snprintf(message_with_username, sizeof(message_with_username), "\033[1;36m%.100s: \033[0m%.900s", clients[i].username, buffer);
          broadcast_message(clients, MAX_CLIENTS, message_with_username, sd);
        }
      }
    }
  }

  printf("\033[1;31mShutting down server...\033[0m\n");
  for (i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i].fd > 0) {
      close(clients[i].fd);
    }
  }
  close(server_fd);
  return 0;
}
