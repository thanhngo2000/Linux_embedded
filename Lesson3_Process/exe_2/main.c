#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NUMBER_ARGUMENTS 2

int main(int argc, char *argv[])
{
    if (argc > NUMBER_ARGUMENTS)
    {
        printf("Argument must be %d\n", NUMBER_ARGUMENTS);
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("Fork process Failed");
        return EXIT_FAILURE;
    }

    if (pid == 0) // child process
    {
        printf("In child process\n");
        if (strcmp(argv[1], "1") == 0)
        {
            printf("Run ls command\n");
            execlp("ls", "ls", "-l", NULL);
            printf("continue child process\n"); // not print this line
        }
        else if (strcmp(argv[1], "2") == 0)
        {
            printf("Run date command\n");
            execlp("date", "date", NULL);
            printf("continue child process\n"); // not print this line
        }
        else
        {
            printf("Invalid argument. Please use 1 or 2\n");
            return EXIT_FAILURE;
        }
    }
    else if (pid > 0)
    {
        sleep(1);
        printf("In parent process\n"); // still print this line
    }

    return EXIT_SUCCESS;
}
