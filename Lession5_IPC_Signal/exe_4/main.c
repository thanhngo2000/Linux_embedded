#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

//use sigaction will better than signal because it still catch handler even SIGTSTP is ignored

void signal_handler(int signum) {
    printf("\nReceived signal %d\n", signum);
    printf("SIGTSTP ignored\n");
}


int main()
{
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;// no special flags

    // empty set
    sigemptyset(&sa.sa_mask);

    //add SIGTSTP to set
    if (sigaction(SIGTSTP, &sa, NULL) == -1)
    {
        printf("Cannot handle SIGTSTP\n");
    }
    else
    {
        if (sigismember(&sa.sa_mask, SIGTSTP) == 1 ) {
			printf("SIGTSTP exist\n");
		} else if (sigismember(&sa.sa_mask, SIGTSTP) == 0) {
			printf("SIGTSTP does not exist\n");
		}
    }
    while(1)
    {
        pause();
    }
    return EXIT_SUCCESS;
}