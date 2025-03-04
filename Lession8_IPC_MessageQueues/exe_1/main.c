#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <errno.h>

#define MAX_SIZE 256
#define QUEUE_NAME "/my_message_queue"

int main()
{
    struct mq_attr attr;
    char buffer[MAX_SIZE];
    pid_t pid;

    mqd_t mq;

    // set up attributes for queues
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;        // max message in queue
    attr.mq_msgsize = MAX_SIZE; // max size for each messafe
    attr.mq_curmsgs = 0;

    // create message queue
    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);
    // printf("mq: %d\n", (int)mq);
    if (mq == (mqd_t)-1)
    {
        perror("mq open failed");
        exit(EXIT_FAILURE);
    }

    // create process
    pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        // child process
        // receive message
        if (mq_receive(mq, buffer, MAX_SIZE, NULL) == -1)
        {
            perror("mq receive failed");
            exit(EXIT_FAILURE);
        }
        printf("Received message: %s\n", buffer);
    }
    else
    {
        // parent process
        //  const char *message = "Hello from parent";
        snprintf(buffer, MAX_SIZE, "Hello from parent");
        if (mq_send(mq, buffer, strlen(buffer) + 1, 0 == -1))
        {
            perror("mq send failed");
            exit(EXIT_FAILURE);
        }
        sleep(1);
        wait(NULL);
    }

    // close message queue
    mq_close(mq);

    // delete message queue
    mq_unlink(QUEUE_NAME);

    return EXIT_SUCCESS;
}