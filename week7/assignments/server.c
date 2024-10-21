#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int game_board[3][3]; // Game board

// Initialize the game board
void init_game_board() {
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      game_board[i][j] = 0;
}

// Print the game board on the server
void print_game_board() {
  printf("\nGame Board:\n");
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (game_board[i][j] == 1)
        printf("\033[34m X \033[0m");
      else if (game_board[i][j] == 2)
        printf("\033[31m O \033[0m");
      else
        printf(" . ");
    }
    printf("\n");
  }
}

// Send TURN notification to the current player
void notify_turn(int client_sock) {
  send(client_sock, "TURN", 5, 0);
}

void send_game_board(int client1_sock, int client2_sock) {
  char buffer[BUFFER_SIZE] = {0};
  int offset = 0;

  // Convert the game board to a string
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (game_board[i][j] == 1)
        offset += snprintf(buffer + offset, BUFFER_SIZE - offset, "\033[34m X \033[0m");
      else if (game_board[i][j] == 2)
        offset += snprintf(buffer + offset, BUFFER_SIZE - offset, "\033[31m O \033[0m");
      else
        offset += snprintf(buffer + offset, BUFFER_SIZE - offset, " . ");
    }
    offset += snprintf(buffer + offset, BUFFER_SIZE - offset, "\n");
  }

  send(client1_sock, buffer, BUFFER_SIZE, 0);
  send(client2_sock, buffer, BUFFER_SIZE, 0);
}

int check_winner() {
  for (int i = 0; i < 3; i++) {
    if (game_board[i][0] == game_board[i][1] && game_board[i][1] == game_board[i][2] && game_board[i][0] != 0)
      return game_board[i][0];
    if (game_board[0][i] == game_board[1][i] && game_board[1][i] == game_board[2][i] && game_board[0][i] != 0)
      return game_board[0][i];
  }
  if (game_board[0][0] == game_board[1][1] && game_board[1][1] == game_board[2][2] && game_board[0][0] != 0)
    return game_board[0][0];
  if (game_board[0][2] == game_board[1][1] && game_board[1][1] == game_board[2][0] && game_board[0][2] != 0)
    return game_board[0][2];
  return 0;
}

int main() {
  int server_fd, client1_fd, client2_fd;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  char buffer[BUFFER_SIZE] = {0};
  int move_count = 0;
  int current_player = 1; // Player 1 starts

  // 2.1 Set up connection
  // Create socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("Socket failed");
    exit(EXIT_FAILURE);
  }

  // Configure server address
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // Bind socket to address
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Bind failed");
    exit(EXIT_FAILURE);
  }

  // Listen for connections
  if (listen(server_fd, 2) < 0) {
    perror("Listen failed");
    exit(EXIT_FAILURE);
  }

  printf("\033[33mWaiting for two players to connect...\033[0m\n");

  // Accept connection from player 1 and player 2
  if ((client1_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
    perror("Accept client 1 failed");
    exit(EXIT_FAILURE);
  }
  printf("\033[32mPlayer 1 has connected.\033[0m\n");

  if ((client2_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
    perror("Accept client 2 failed");
    exit(EXIT_FAILURE);
  }
  printf("\033[32mPlayer 2 has connected.\033[0m\n");

  // Initialize the game board
  init_game_board();

  // Game loop
  while (1) {
    int row, col, winner;

    // 2.2 Alternate turns
    // Notify the current player of their turn
    if (current_player == 1) {
      notify_turn(client1_fd);
      recv(client1_fd, buffer, BUFFER_SIZE, 0);
    } else {
      notify_turn(client2_fd);
      recv(client2_fd, buffer, BUFFER_SIZE, 0);
    }

    sscanf(buffer, "%d %d", &row, &col);

    // 2.3 Validate move
    // Check if the move is valid
    if (game_board[row][col] == 0) {
      game_board[row][col] = current_player;
      move_count++;
    } else {
      // Send invalid move notification
      if (current_player == 1) {
        send(client1_fd, "INVALID", 8, 0);
      } else {
        send(client2_fd, "INVALID", 8, 0);
      }
      continue;
    }

    print_game_board();

    // 2.3 Update game board
    // Send the updated game board to both clients
    send_game_board(client1_fd, client2_fd);

    // 2.4 Check for win or draw
    winner = check_winner();
    if (winner > 0) {
      // Send win notification
      snprintf(buffer, BUFFER_SIZE, "WIN %d", winner);
      send(client1_fd, buffer, BUFFER_SIZE, 0);
      send(client2_fd, buffer, BUFFER_SIZE, 0);
      printf("\033[32mPlayer %d wins!\033[0m\n", winner);
      break;
    }

    // Check for draw
    if (move_count == 9) {
      send(client1_fd, "DRAW", BUFFER_SIZE, 0);
      send(client2_fd, "DRAW", BUFFER_SIZE, 0);
      printf("\033[33mThe game is a draw!\033[0m\n");
      break;
    }

    // Switch player
    current_player = (current_player == 1) ? 2 : 1;
  }

  // Close connections
  close(client1_fd);
  close(client2_fd);
  close(server_fd);
  return 0;
}
