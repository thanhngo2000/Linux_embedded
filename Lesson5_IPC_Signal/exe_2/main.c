#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int count =0;

void signal_handler(int signal)
{
    // printf("Receive SIGALRM signal after 1 second\n");
    count++;
    printf("Timer: %d\n", count);
    if (count ==10)
    {
        exit(EXIT_SUCCESS);
    }

}

int main()
{
    //sign alarm for signal handler
    // signal(SIGALRM, signal_handler);
    if (signal(SIGALRM, signal_handler) == SIG_ERR) {
        perror("Error registering signal handler");
        exit(EXIT_FAILURE);
    }


    // set up timer 1s
    alarm(1);

    printf("Waiting for signal alarm\n");

    while (1)
    {

        pause();// wait for signal
        alarm(1); //set up alarm again
    }


    return EXIT_SUCCESS;
}
