#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
  int sock = 0;
  struct sockaddr_in serv_addr;
  char buffer[BUFFER_SIZE] = {0};
  int row, col;
  int input_status; // Variable to check input status

  // 3.1 Set up connection
  // Create TCP socket
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\033[31mSocket creation error\033[0m\n");
  }

  // Configure server address
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  // Convert server address to binary form
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    printf("\033[31mInvalid address/ Address not supported\033[0m\n");
    return -1;
  }

  // Connect to server
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("\033[31mConnection Failed\033[0m\n");
    return -1;
  }

  while (1) {
    // 3.2 Receive turn notification
    recv(sock, buffer, BUFFER_SIZE, 0);

    if (strncmp(buffer, "TURN", 4) == 0) {
      do {
        printf("\033[34mEnter your move (row and column): \033[0m");
        input_status = scanf("%d %d", &row, &col);

        if (input_status != 2) {
          printf("\033[31mInvalid input, please enter two integers!\033[0m\n");

          while (getchar() != '\n');
        } else if (row < 0 || row > 2 || col < 0 || col > 2) {
          printf("\033[33mPlease enter values for row and column between 0 and 2.\033[0m\n");
          input_status = 0;
        }

      } while (input_status != 2);

      // Create MOVE message to send to server
      snprintf(buffer, BUFFER_SIZE, "%d %d", row, col);
      send(sock, buffer, strlen(buffer), 0);

    } else if (strncmp(buffer, "INVALID", 7) == 0) {
      // 3.2 Receive invalid move notification
      printf("\033[31mInvalid move, please try again!\033[0m\n");
    } else if (strncmp(buffer, "WIN", 3) == 0) {
      // 3.3 Receive win notification
      printf("\033[1m\033[31m Player %c has won!\033[0m\n", buffer[4]);
      break;
    } else if (strncmp(buffer, "DRAW", 4) == 0) {
      // 3.3 Receive draw notification
      printf("\033[33m The game is a draw!\033[0m\n");
      break;
    } else {
      // 3.3 Receive game board update
      printf("\033[32mUpdated game board:\033[0m\n%s", buffer);
    }
  }

  // Close connection
  close(sock);
  return 0;
}
