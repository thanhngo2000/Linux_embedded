#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

#define BUFFER_SIZE 10

// int count_time =0;
int count_product =0;

int in_num =0;//produce postion
int out_num =0;//consume position

int buff_producer[BUFFER_SIZE];
int buff_consumer[BUFFER_SIZE];

static void *producer(void *args) 
{
    // pthread_mutex_lock(&mutex_lock);
    
    for (int i=0; i < BUFFER_SIZE; i++) //not: cannot use I to trace value
    {
        pthread_mutex_lock(&mutex_lock);

        //wait if buffer is full
        while (count_product == BUFFER_SIZE)
        {
            // send sig for consumer thread
            pthread_cond_wait(&cond_var, &mutex_lock);
        }
        
        //produce data from 1 to 10
        int data = rand() % 10 +1;
        // printf("Produced: %d\n", data);
        buff_producer[in_num] = data;
        printf("Produced: %d\n",data);
        count_product++;
        in_num = in_num +1;//update produce number

        //send signal to consumer
        pthread_cond_signal(&cond_var);
        pthread_mutex_unlock(&mutex_lock);
        sleep(1);
    }
    pthread_exit(NULL);
}
static void *consumer(void *args) 
{

    for (int i=0; i< BUFFER_SIZE; i++)
    {
        pthread_mutex_lock(&mutex_lock);

        //wait if buffer empty
        while (count_product == 0) {
            pthread_cond_wait(&cond_var, &mutex_lock);
        }

        //get data from buffer
        int data = buff_producer[out_num];
        buff_consumer[out_num] = data;
        // int item = buff_producer[count_product];
        count_product--;
        // printf("Consumed: %d\n", buff_consumer[i]);
        printf("Consumed: %d\n", data);
        out_num = out_num + 1 ; //update number out consume

        //send signal to producer
        pthread_cond_signal(&cond_var);
        pthread_mutex_unlock(&mutex_lock);
        sleep(1);
    }
    
    pthread_exit(NULL);

}

int main()
{
    srand(time(NULL));

    pthread_t producer_thread, consumer_thread;

    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    printf("Consumer final buff: ");
    for (int i=0; i <BUFFER_SIZE; i++)
    {
        printf("%d ", buff_consumer[i]);
    }
    printf("\n");

    return EXIT_SUCCESS;
}

