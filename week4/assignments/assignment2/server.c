// server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_QUESTIONS 10

char *questions[MAX_QUESTIONS] = {
    "1 + 1 = ?\nA. 1\nB. 2\nC. 3\nD. 4",
    "What is the capital of France?\nA. London\nB. Paris\nC. Berlin\nD. Madrid",
    "5 * 5 = ?\nA. 20\nB. 25\nC. 30\nD. 35",
    "What is the largest planet?\nA. Earth\nB. Mars\nC. Jupiter\nD. Venus",
    "What is the boiling point of water?\nA. 90°C\nB. 100°C\nC. 110°C\nD. 120°C",
    "Which language is used for web development?\nA. Python\nB. HTML\nC. C++\nD. Java",
    "Who developed the theory of relativity?\nA. Newton\nB. Einstein\nC. Galileo\nD. Bohr",
    "What is the capital of Japan?\nA. Seoul\nB. Tokyo\nC. Kyoto\nD. Osaka",
    "What is 10 / 2 = ?\nA. 3\nB. 5\nC. 7\nD. 9",
    "Which element is denoted by 'O' in chemistry?\nA. Oxygen\nB. Hydrogen\nC. Helium\nD. Carbon"
};

char correct_answers[MAX_QUESTIONS] = {'B', 'B', 'B', 'C', 'B', 'B', 'B', 'B', 'B', 'A'};

// Signal handler to prevent zombie processes
void sigchld_handler(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_client(int connfd) {
    char buffer[BUFFER_SIZE];
    int score = 0;

    for (int i = 0; i < MAX_QUESTIONS; i++) {
        if (questions[i] != NULL) {
            send(connfd, questions[i], strlen(questions[i]), 0);
        } else {
            snprintf(buffer, sizeof(buffer), "Question %d does not exist.\n", i+1);
            send(connfd, buffer, strlen(buffer), 0);
        }

        memset(buffer, 0, BUFFER_SIZE);
        read(connfd, buffer, sizeof(buffer));

        if (buffer[0] == correct_answers[i]) {
            score++;
        }
    }

    snprintf(buffer, sizeof(buffer), "Your final score is: %d/%d\n", score, MAX_QUESTIONS);
    send(connfd, buffer, strlen(buffer), 0);

    printf("Client's score: %d\n", score);

    close(connfd);
    exit(0);
}

int main() {
    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pid_t pid;

    // Create a socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Define server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(listenfd, 5) < 0) {
        perror("Listen failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // Handle SIGCHLD to prevent zombie processes
    signal(SIGCHLD, sigchld_handler);

    printf("Server is running on port %d...\n", PORT);

    while (1) {
        // Accept incoming connection
        if ((connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len)) < 0) {
            perror("Accept failed");
            continue;
        }

        // Fork a child process to handle the client
        if ((pid = fork()) == 0) {

            close(listenfd);
            handle_client(connfd);
        } else if (pid > 0) {

            close(connfd);
        } else {

            perror("Fork failed");
            close(connfd);
        }
    }

    return 0;
}
