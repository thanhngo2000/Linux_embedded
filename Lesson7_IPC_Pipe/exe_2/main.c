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
    int pipefd_parent_2_child[2];
    int pipefd_child_2_child[2];
    pid_t pid1, pid2;
    char buffer[BUFFER_SIZE];

    // create pipe
    if (pipe(pipefd_parent_2_child) == -1)
    {
        perror("pipe 1 failed");
        exit(EXIT_FAILURE);
    }

    if (pipe(pipefd_child_2_child) == -1)
    {
        perror("pipe 2 failed");
        exit(EXIT_FAILURE);
    }

    pid1 = fork();
    if (pid1 < 0)
    {
        perror("fork 1 failed");
        exit(EXIT_FAILURE);
    }
    else if (pid1 == 0)
    {
        printf("Child1 process\n");
        // block write pipe1
        close(pipefd_parent_2_child[HEAD_WRITE]); // close write

        // process read pipe1
        read(pipefd_parent_2_child[HEAD_READ], buffer, sizeof(buffer)); // read from pipe
        printf("Child received: %s\n", buffer);
        close(pipefd_parent_2_child[HEAD_READ]); // close read

        // process write pipe2
        close(pipefd_child_2_child[HEAD_READ]); // close read
        strcat(buffer, " and child");
        write(pipefd_child_2_child[HEAD_WRITE], buffer, sizeof(buffer) + 1); // write to pipe
        close(pipefd_child_2_child[HEAD_WRITE]);                             // close write

        exit(EXIT_SUCCESS);
    }

    pid2 = fork();
    if (pid2 < 0)
    {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    else if (pid2 == 0)
    {
        printf("Child2 process\n");

        // read pipe2
        close(pipefd_child_2_child[HEAD_WRITE]);                       // close write
        read(pipefd_child_2_child[HEAD_READ], buffer, sizeof(buffer)); // read from pipe
        printf("Child2 received: %s\n", buffer);
        close(pipefd_child_2_child[HEAD_READ]); // close read

        exit(EXIT_SUCCESS);
    }
    else
    {
        printf("Parent process\n");

        // write pipe1
        close(pipefd_parent_2_child[HEAD_READ]); // close read
        const char *message = "Hello from parent";
        write(pipefd_parent_2_child[HEAD_WRITE], message, strlen(message) + 1);
        close(pipefd_parent_2_child[HEAD_WRITE]);

        wait(NULL);
    }

    return EXIT_SUCCESS;
}
