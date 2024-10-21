#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int game_board[3][3]; // Bảng trò chơi

// Khởi tạo bảng trò chơi
void init_game_board() {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            game_board[i][j] = 0;
}

// In bảng trò chơi lên server
void print_game_board() {
    printf("\nBảng trò chơi:\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (game_board[i][j] == 1)
                printf(" X ");
            else if (game_board[i][j] == 2)
                printf(" O ");
            else
                printf(" . ");
        }
        printf("\n");
    }
}

// Gửi thông báo TURN cho người chơi hiện tại
void notify_turn(int client_sock) {
    send(client_sock, "TURN", 5, 0);
}

// Gửi bảng trò chơi cập nhật cho cả hai người chơi
void send_game_board(int client1_sock, int client2_sock) {
    char buffer[BUFFER_SIZE] = {0};
    int offset = 0;

    // Duyệt qua bảng trò chơi và chuyển thành chuỗi
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (game_board[i][j] == 1)
                offset += snprintf(buffer + offset, BUFFER_SIZE - offset, " X ");
            else if (game_board[i][j] == 2)
                offset += snprintf(buffer + offset, BUFFER_SIZE - offset, " O ");
            else
                offset += snprintf(buffer + offset, BUFFER_SIZE - offset, " . ");
        }
        offset += snprintf(buffer + offset, BUFFER_SIZE - offset, "\n");
    }

    // Gửi bảng trò chơi cho cả hai client
    send(client1_sock, buffer, BUFFER_SIZE, 0);
    send(client2_sock, buffer, BUFFER_SIZE, 0);
}

// Kiểm tra người thắng
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
    return 0; // Không có người thắng
}

// Hàm chính của server
int main() {
    int server_fd, client1_fd, client2_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    int move_count = 0;
    int current_player = 1; // Người chơi 1 bắt đầu

    // 2.1 Thiết lập kết nối
    // Tạo socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Cấu hình địa chỉ
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Gắn socket với địa chỉ
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối
    if (listen(server_fd, 2) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Chờ kết nối từ hai người chơi...\n");

    // Chấp nhận kết nối client 1 và client 2
    if ((client1_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("Accept client 1 failed");
        exit(EXIT_FAILURE);
    }
    printf("Người chơi 1 đã kết nối.\n");

    if ((client2_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("Accept client 2 failed");
        exit(EXIT_FAILURE);
    }
    printf("Người chơi 2 đã kết nối.\n");

    // Khởi tạo bảng trò chơi
    init_game_board();

    // Vòng lặp trò chơi
    while (1) {
        int row, col, winner;

        // 2.2 Luân phiên lượt chơi
        // Thông báo lượt cho người chơi hiện tại
        if (current_player == 1) {
            notify_turn(client1_fd);
            recv(client1_fd, buffer, BUFFER_SIZE, 0); // Nhận nước đi từ Người chơi 1
        } else {
            notify_turn(client2_fd);
            recv(client2_fd, buffer, BUFFER_SIZE, 0); // Nhận nước đi từ Người chơi 2
        }

        sscanf(buffer, "%d %d", &row, &col);

        // 2.3 Xác thực nước đi
        // Kiểm tra nước đi hợp lệ
        if (game_board[row][col] == 0) {
            game_board[row][col] = current_player;
            move_count++;
        } else {
            // Gửi thông báo nước đi không hợp lệ
            if (current_player == 1) {
                send(client1_fd, "INVALID", 8, 0);
            } else {
                send(client2_fd, "INVALID", 8, 0);
            }
            continue; // Bỏ qua và yêu cầu người chơi gửi lại nước đi
        }

        // In bảng trò chơi lên server
        print_game_board();

        // 2.3 Cập nhật bảng trò chơi
        // Gửi bảng trò chơi đã cập nhật cho cả hai client
        send_game_board(client1_fd, client2_fd);

        // 2.4 Kiểm tra thắng hoặc hòa
        winner = check_winner();
        if (winner > 0) {
            // Gửi thông báo kết quả thắng
            snprintf(buffer, BUFFER_SIZE, "WIN %d", winner);
            send(client1_fd, buffer, BUFFER_SIZE, 0);
            send(client2_fd, buffer, BUFFER_SIZE, 0);
            break;
        }

        // Kiểm tra nếu hòa
        if (move_count == 9) {
            send(client1_fd, "DRAW", BUFFER_SIZE, 0);
            send(client2_fd, "DRAW", BUFFER_SIZE, 0);
            break;
        }

        // Luân phiên người chơi
        current_player = (current_player == 1) ? 2 : 1;
    }

    // Đóng kết nối
    close(client1_fd);
    close(client2_fd);
    close(server_fd);
    return 0;
}
