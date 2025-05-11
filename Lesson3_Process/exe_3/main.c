#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

void handle_sigusr1(int signo)
{
    printf("Received signal SIGUSR1.\n");
}

int main()
{
    pid_t pid = fork();
    if (pid < 0)
    {
        printf("Fork failed");
        return EXIT_FAILURE;
    }

    else if (pid == 0) // child process
    {
        printf("In process child\n");
        sleep(1);

        if (signal(SIGUSR1, handle_sigusr1) == SIG_ERR)
        {
            perror("signal failed");
            exit(EXIT_FAILURE);
        }
        pause(); // wait for signal
        printf("Finished child process after signal SIGUSR1\n");
    }
    else // parent process
    {
        printf("In process parent\n");
        sleep(2); // do process parent

        // send user signal to child
        printf("Send signal SIGUSR1 to child %d\n", pid);
        kill(pid, SIGUSR1);

        wait(NULL); // wait until any child finish
        printf("Finished parent process\n");
    }
    return EXIT_SUCCESS;
}
