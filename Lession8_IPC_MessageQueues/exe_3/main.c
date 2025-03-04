#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

#include <fcntl.h>
#include <errno.h>

#define MAX_SIZE 256
#define MESSAGE_QUEUE_NAME "/message_queue"

void convert_to_uppercase(char *str)
{
    while (*str)
    {
        *str = toupper((unsigned char)*str); // change to uppercase
        str++;
    }
}

int main()
{
    struct mq_attr attr;
    pid_t pid, pid1;

    mqd_t mq;
    char message[MAX_SIZE];

    // set up attributes for queues
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;        // max message in queue
    attr.mq_msgsize = MAX_SIZE; // max size for each message
    attr.mq_curmsgs = 0;

    // create message queue send
    mq = mq_open(MESSAGE_QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);
    if (mq == (mqd_t)-1)
    {
        perror("mq send open failed");
        exit(EXIT_FAILURE);
    }

    // create process
    pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        mq_close(mq);
        mq_unlink(MESSAGE_QUEUE_NAME);
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        // child 1 process
        // receive message
        ssize_t num_bytes_read = mq_receive(mq, message, MAX_SIZE, NULL);
        if (num_bytes_read >= 0)
        {
            message[num_bytes_read] = '\0';
            printf("child receive message from parent: %s\n", message);

            // Change up uppercase
            convert_to_uppercase(message);

            // snprintf(message, sizeof(message), "%d", count);

            // send message tp child 2
            if (mq_send(mq, message, strlen(message) + 1, 0) == -1)
            {
                perror("mq send child1 failed");
            }
        }
        else
        {
            perror("mq receive in child1 failed");
        }
        exit(EXIT_SUCCESS);
    }
    else
    {
        pid1 = fork();

        if (pid1 < 0)
        {
            perror("fork failed");
            mq_close(mq);
            mq_unlink(MESSAGE_QUEUE_NAME);
            exit(EXIT_FAILURE);
        }
        else if (pid1 == 0)
        {
            // child process 2
            // receive message from parent
            ssize_t num_bytes_read = mq_receive(mq, message, MAX_SIZE, NULL);
            if (num_bytes_read >= 0)
            {
                message[num_bytes_read] = '\0';
                printf("child 2 receive message from child1: %s\n", message);
            }
            else
            {
                perror("mq receive in child2 failed");
            }

            exit(EXIT_SUCCESS);
        }
        else
        {
            // parent process
            // send request messsage from parent to child
            snprintf(message, sizeof(message), "Hello from parent");
            if (mq_send(mq, message, strlen(message) + 1, 0) == -1)
            {
                perror("mq send in parent failed");
            }
            wait(NULL);
        }
    }

    // close message queue
    mq_close(mq);

    // delete message queue
    mq_unlink(MESSAGE_QUEUE_NAME);

    return EXIT_SUCCESS;
}
