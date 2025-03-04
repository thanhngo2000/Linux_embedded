#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#define ARRAY_SIZE 100

int numbers[ARRAY_SIZE];
int numbers_even[ARRAY_SIZE];
int numbers_odd[ARRAY_SIZE];

int count_odd =0;
int count_even =0;
int count =0;

static void *count_odd_handler (void *args)
{
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        if (numbers[i] % 2!= 0) {
            int data = numbers[i];
            numbers_odd[count_odd]= data;

            count_odd++;
            count++;
        }
    }
    pthread_exit(NULL);
}

static void *count_even_handler (void *args)
{
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        if (numbers[i] % 2== 0) {
            int data = numbers[i];
            numbers_even[count_even]= data;

            count_even++;
            count++;
        }
    }
    pthread_exit(NULL);
}

int main() {
    

    // create seed
    srand(time(NULL));
    // create array
    for (int i = 0; i < ARRAY_SIZE; i++) {
        numbers[i] = rand() % 100 + 1; // create random number
    }
    
    pthread_t thread_odd, thread_even;
    int ret;
    //create new thread 1
    if (ret = pthread_create(&thread_odd,NULL , &count_odd_handler, NULL))
    {
        printf("pthread_create() 1 error number=%d\n", ret);
        return EXIT_FAILURE;
    }
    
    if (ret = pthread_create(&thread_even,NULL , &count_even_handler, NULL))
    {
        printf("pthread_create() 2 error number=%d\n", ret);
        return EXIT_FAILURE;
    }


    //wait threads finish
    pthread_join(thread_odd, NULL);
    pthread_join(thread_even, NULL);


    //print
    printf("Array :\n");
    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("%d ", numbers[i]);
    }
    printf("\n");

    printf("Array odd :\n");
    for (int i = 0; i < count_odd; i++) {
        printf("%d ", numbers_odd[i]);
    }
    printf("\n");
    printf("Count odd numers: %d\n", count_odd);

    printf("Array event :\n");
    for (int i = 0; i < count_even; i++) {
        printf("%d ", numbers_even[i]);
    }
    printf("\n");
    printf("Count even numers: %d\n", count_even);


    return EXIT_SUCCESS;
}