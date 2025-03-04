#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
void signal_handler(int signum)
{
    printf("SIGINT received\n");
}
int main()
{
    printf("Get PID: %d\n", getpid());
    // create signal action

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    // sign handler for signal
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("signal failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        if (sigismember(&sa.sa_mask, SIGINT) == 1)
        {
            printf("SIGINT exist\n");
        }
        else if (sigismember(&sa.sa_mask, SIGINT) == 0)
        {
            printf("SIGINT does not exist\n");
        }
    }

    fd_set readfds; // set readfds
    int max_fd = 0;

    while (1)
    {
        FD_ZERO(&readfds);              // clear readfds
        FD_SET(STDIN_FILENO, &readfds); // add stdin to readfds
        max_fd = STDIN_FILENO;

        // wait signal for data from stdin
        int result;
        do
        {
            result = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        } while (result == -1 && errno == EINTR); // if interrupted by SIGINT for AITNTR, it will call select again

        // check data from stdin
        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            char buffer[100];
            ssize_t bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer));

            if (bytes_read > 0)
            {
                buffer[bytes_read] = '\0';
                printf("\r\033[K"); // delete currrent line in terminal
                /*\r: put ponter to begin of line
                \033[K: ANSI code to delete contents from pointer to end of line
                */
                printf("Received data: %s \n", buffer);
            }
        }
    }
    return EXIT_SUCCESS;
}
