#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define ARRAY_SIZE 1000000
#define NUM_THREADS 4

long long g_sum[NUM_THREADS];
int array[ARRAY_SIZE];
pthread_mutex_t mutex;

void *cal_sum(void *arg)
{
    int thread_id= *(int *)arg;
    int size = ARRAY_SIZE/NUM_THREADS;
    int start = thread_id * size;
    int end = (thread_id +1)*size;

    //cal sum from start value to end value
    long long sum =0;
    for (int i=start; i<end; i++)
    {
        sum += array[i];
    }

    //add sum to global sum with lonk mutex
    pthread_mutex_lock(&mutex);
    g_sum[thread_id] = sum;
    pthread_mutex_unlock(&mutex);

    free(arg); //free thread_id memory

    return EXIT_SUCCESS;

}

int main()
{
    pthread_t threads[NUM_THREADS];
    pthread_mutex_init(&mutex, NULL);

    //create time for random number
    srand(time(NULL));

    //create random number
    for (int i=0; i<ARRAY_SIZE; i++){
        array[i] = rand();
    }

    //create threads
    for (int i=0; i < NUM_THREADS; i++){
        int *thread_id = malloc(sizeof(int));
        *thread_id = i;
        pthread_create(&threads[i], NULL, cal_sum, (void *)thread_id);
    }

    //wait for thread end
    for (int i=0; i < NUM_THREADS; i++){
        pthread_join(threads[i], NULL);
    }

    //cal global sum
    long long total_sum =0;
    for (int i=0; i < NUM_THREADS; i++){
        total_sum += g_sum[i];
    }
    
    printf("Total sum of all threads: %lld\n", total_sum);

    pthread_mutex_destroy(&mutex);

    return EXIT_SUCCESS;
}