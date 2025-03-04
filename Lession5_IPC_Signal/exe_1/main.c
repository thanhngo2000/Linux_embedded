#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int count;

void signal_handler(int signal)
{
    printf("Received signal: %d number %d\n", signal, count+1);
    count++;
    if (count ==3)
    {
        exit(EXIT_SUCCESS);// exit the program
    }
}

int main()
{
    //sign signal SIGINT for signal handler
    // signal(SIGINT, signal_handler);
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("Error registering signal handler");
        exit(EXIT_FAILURE);
    }

    printf("Prosess ID: %d\n", getpid());
    printf("Press Ctrl+C to send SIGINT\n");

    while(1)
    {
        pause();
    }


    return EXIT_SUCCESS;
}