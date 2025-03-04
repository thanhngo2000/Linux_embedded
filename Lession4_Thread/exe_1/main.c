#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

static void *thread_handle1(void *args)
{
    // get thread id in function
    pthread_t thread_id = pthread_self();
    printf("Thread id %lu: Hello from thread 1\n", (unsigned long)thread_id);
    pthread_exit(NULL);
}

static void *thread_handle2(void *args)
{
    // get thread id in function
    pthread_t thread_id = pthread_self();
    printf("Thread id %lu: Hello from thread 2\n", (unsigned long)thread_id);
    pthread_exit(NULL);
}

int main()
{
    pthread_t thread_id1, thread_id2;
    int ret;
    // get thread id in main
    pthread_t main_thread_id = pthread_self();
    printf("Thread ID from main: %lu\n", (unsigned long)main_thread_id);

    // create new thread 1
    if (ret = pthread_create(&thread_id1, NULL, &thread_handle1, NULL))
    {
        printf("pthread_create() error number=%d\n", ret);
        return EXIT_FAILURE;
    }
    // pthread_join(thread_id1, NULL);

    if (ret = pthread_create(&thread_id2, NULL, &thread_handle2, NULL))
    {
        printf("pthread_create() error number=%d\n", ret);
        return EXIT_FAILURE;
    }

    // wait threads finish
    pthread_join(thread_id1, NULL);
    pthread_join(thread_id2, NULL);

    return EXIT_SUCCESS;
}