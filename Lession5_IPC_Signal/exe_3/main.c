#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int count = 0;

void signal_handler(int signal)
{
    printf("Received signal from parent\n");
    count++;
    if (count == 5)
    {
        exit(EXIT_SUCCESS); // Terminate the child process
    }
}

int main()
{
    pid_t pid = fork();

    if (pid < 0)
    {
        printf("Fork failed\n");
        return EXIT_FAILURE;
    }
    else if (pid == 0)
    {

        printf("Child process\n");
        if (signal(SIGUSR1, signal_handler) == SIG_ERR)
        {
            printf("Cannot handle SIGUSR1\n");
            exit(EXIT_FAILURE);
        }
        while (1)
        {
            // printf("Child waiting\n");
            pause();
        }
    }
    else
    {
        printf("Parent process\n");
        printf("Process parent will send signal after each 5 seconds to child process pid: %d\n", pid);
        int count_parent = 0;
        while (count_parent < 5)
        {
            if (count == 5)
            {
                exit(EXIT_SUCCESS); // Terminate the child process
            }
            sleep(2);
            kill(pid, SIGUSR1); // Send signal to child process
            count_parent++;
        }
    }
    return EXIT_SUCCESS;
}