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

    // 3.1 Thiết lập kết nối
    // Tạo socket TCP
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return -1;
    }

    // Cấu hình địa chỉ của server
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Chuyển địa chỉ server thành định dạng nhị phân
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported\n");
        return -1;
    }

    // Kết nối đến server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed\n");
        return -1;
    }

    while (1) {
        // 3.2 Nhận thông báo lượt chơi
        recv(sock, buffer, BUFFER_SIZE, 0); // Chờ nhận thông báo từ server

        if (strncmp(buffer, "TURN", 4) == 0) {
            // Nếu đến lượt client
            printf("Nhập nước đi của bạn (hàng và cột): ");
            scanf("%d %d", &row, &col); // Nhập nước đi

            // Tạo thông điệp MOVE để gửi đến server
            snprintf(buffer, BUFFER_SIZE, "%d %d", row, col);
            send(sock, buffer, strlen(buffer), 0); // Gửi nước đi đến server
        } else if (strncmp(buffer, "INVALID", 7) == 0) {
            // 3.2 Nhận thông báo nước đi không hợp lệ
            printf("Nước đi không hợp lệ, hãy thử lại!\n");
        } else if (strncmp(buffer, "WIN", 3) == 0) {
            // 3.3 Nhận kết quả thắng
            printf("Người chơi %c đã thắng!\n", buffer[4]);
            break;
        } else if (strncmp(buffer, "DRAW", 4) == 0) {
            // 3.3 Nhận kết quả hòa
            printf("Trò chơi hòa!\n");
            break;
        } else {
            // 3.3 Nhận cập nhật trạng thái bảng trò chơi
            printf("Cập nhật bảng trò chơi:\n%s", buffer); // Hiển thị bảng trò chơi được cập nhật
        }
    }

    // Đóng kết nối sau khi trò chơi kết thúc
    close(sock);
    return 0;
}
