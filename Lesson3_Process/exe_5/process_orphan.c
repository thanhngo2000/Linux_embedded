#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    pid_t pid = fork();

    if (pid < 0)
    {
        printf("fork failed\n");
        return EXIT_FAILURE;
    }
    else if (pid == 0)
    {
        // child process
        printf("Child process\n");
        while (1)
            ;
        printf("child process end.\n");
    }
    else
    {
        // Parent process
        printf("Parent process\n");
        sleep(2); // wait 2 second
        printf("parent process end.\n");
        exit(0);
    }
    return EXIT_SUCCESS;
}