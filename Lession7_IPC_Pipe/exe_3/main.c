#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>

#define BUFFER_SIZE 100

#define HEAD_WRITE 1
#define HEAD_READ 0

int main()
{
    int pipefd[2];
    pid_t pid;
    char buffer[BUFFER_SIZE];

    //create pipe
    if (pipe(pipefd) == -1)
    {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    else if (pid == 0)
    {
        printf("Child process\n");
        close (pipefd[HEAD_WRITE]); //close write
        read(pipefd[HEAD_READ], buffer, sizeof(buffer));// read from pipe
        printf("Child received: %s\n", buffer);
        size_t length = strlen(buffer) -1; //expect '/0' characters
        printf("Number of characters read: %zu\n",length);
        close(pipefd[HEAD_READ]);//close read
        exit(EXIT_SUCCESS);
    }
    else
    {
        printf("Parent process\n");
        const char *message = "Hello from parent\n";
        close(pipefd[HEAD_READ]); // close read
        write(pipefd[HEAD_WRITE], message, strlen(message)+1);//write to pipe
        close(pipefd[HEAD_WRITE]); // close write

        wait(NULL);
    }

    return EXIT_SUCCESS;
}