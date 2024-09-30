#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork failed\n");
        return 1;
    } else if (pid == 0) {
        printf("This is the child process. My PID is %d.\n", getpid());
        exit(0);
    } else {
        printf("This is the parent process. My child's PID is %d.\n", pid);
        printf("My PID is %d.\n", getpid());
        wait(NULL);
        printf("Parent process has collected the zombie process.\n");
        sleep(30);
    }

    return 0;
}
