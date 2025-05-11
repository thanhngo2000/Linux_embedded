#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <errno.h>


#define MAX_SIZE 256
#define QUEUE_REQUEST_NAME "/queue_request"
#define QUEUE_RESPONSE_NAME "/queue_response"


int main()
{
    struct mq_attr attr;
    char buffer[MAX_SIZE];
    pid_t pid;

    mqd_t mq_request, mq_response;

    char message_request[MAX_SIZE], message_response[MAX_SIZE];

    int count;

    //set up attributes for queues
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10; //max message in queue
    attr.mq_msgsize = MAX_SIZE;// max size for each messafe
    attr.mq_curmsgs = 0;

    //create message queue send
    mq_request = mq_open(QUEUE_REQUEST_NAME, O_CREAT | O_RDWR, 0644, &attr);
    if (mq_request == (mqd_t)-1)
    {
        perror("mqs send open failed");
        exit(EXIT_FAILURE);
    }

    //create message queue count
    mq_response = mq_open(QUEUE_RESPONSE_NAME, O_CREAT | O_RDWR, 0644, &attr);
    if (mq_response == (mqd_t)-1)
    {
        mq_close(mq_request);
        mq_unlink(QUEUE_REQUEST_NAME);
        perror("mqs count open failed");
        exit(EXIT_FAILURE);
    }

    //create process
    pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        mq_close(mq_request);
        mq_close(mq_response);
        mq_unlink(QUEUE_REQUEST_NAME);
        mq_unlink(QUEUE_RESPONSE_NAME);
        exit(EXIT_FAILURE);
    }
    else if (pid ==0)
    {
        //child process
        //receive message
        ssize_t num_bytes_read = mq_receive(mq_request, message_request, MAX_SIZE, NULL);
        if (num_bytes_read >=0)
        {
            message_request[num_bytes_read] = '\0';
            printf("child receive message from parent: %s\n", message_request);

            //conut number byte read
            int count = strlen(message_request);
            snprintf(message_response, sizeof(message_response), "%d", count);

            mq_send(mq_response, message_response, sizeof(message_response), 0) ;
        }
        else
        {
            perror("mq receive (request) failed");
        }
    }
    else
    {
        //parent process
        //send request messsage from parent to child
        snprintf(message_request, sizeof(message_request), "Hello from parent");
        mq_send(mq_request, message_request, sizeof(message_request), 0) ;
   
        // sleep(1);

        ssize_t bytes_read = mq_receive(mq_response, message_response, MAX_SIZE, NULL);
        if (bytes_read >=0)
        {
            message_response[bytes_read] = '\0';
            printf("parent receive character count from child: %s\n", message_response);
        }
        else
        {
            perror("mq receive (response) failed");
        }
    }

    //close message queue
    mq_close(mq_request);
    mq_close(mq_response);

    //delete message queue
    mq_unlink(QUEUE_REQUEST_NAME);
    mq_unlink(QUEUE_RESPONSE_NAME);
    
    return EXIT_SUCCESS;
}