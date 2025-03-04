#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int counter = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; // static mutex

static void *thread_handle1(void *args)
{
    pthread_mutex_lock(&lock);
    printf("Thread 1\n");
    for (int i = 0; i < 1000000; i++)
    {
        counter++;
    }
    printf("Counter from thread 1 %d\n", counter);
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
}

static void *thread_handle2(void *args)
{
    // get thread id in function
    //  printf("Thread 2\n");
    pthread_mutex_lock(&lock);
    printf("Thread 2\n");
    for (int i = 0; i < 1000000; i++)
    {
        counter++;
    }
    printf("Counter from thread 2 %d\n", counter);
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
}

static void *thread_handle3(void *args)
{
    // get thread id in function
    //  printf("Thread 3\n");
    pthread_mutex_lock(&lock);
    printf("Thread 3\n");
    for (int i = 0; i < 1000000; i++)
    {
        counter++;
    }
    printf("Counter from thread 3 %d\n", counter);
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
}

int main()
{
    pthread_t thread_id1, thread_id2, thread_id3;
    int ret;

    // create new thread 1
    if (ret = pthread_create(&thread_id1, NULL, &thread_handle1, NULL))
    {
        printf("pthread_create() 1 error number=%d\n", ret);
        return EXIT_FAILURE;
    }

    if (ret = pthread_create(&thread_id2, NULL, &thread_handle2, NULL))
    {
        printf("pthread_create() 2 error number=%d\n", ret);
        return EXIT_FAILURE;
    }

    if (ret = pthread_create(&thread_id3, NULL, &thread_handle3, NULL))
    {
        printf("pthread_create() 3 error number=%d\n", ret);
        return EXIT_FAILURE;
    }

    // wait threads finish
    pthread_join(thread_id1, NULL);
    pthread_join(thread_id2, NULL);
    pthread_join(thread_id3, NULL);

    printf("Final Counter value: %d\n", counter);

    pthread_mutex_destroy(&lock);

    return EXIT_SUCCESS;
}