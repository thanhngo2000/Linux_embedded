# Excercise

## Exercise 1

- create thread

```bash
pthread_create(&thread_id1, NULL, &thread_handle1, NULL)
```

- Get thread id

```bash
pthread_t thread_id = pthread_self();
```

- wait thread finish

```bash
pthread_join(thread_id1, NULL);
```

**Build and run**

```copy
make
```

```copy
./app
```

**Clean**

```copy
make clean
```

````
## Exercise 2
- create mutex lock
```bash
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
````

- lock mutex

```bash
pthread_mutex_lock(&lock);
```

- unlock mutex

```bash
pthread_mutex_unlock(&lock);
```

- destroy mutex

```bash
pthread_mutex_destroy(&lock);
```

Why we need mutex

- in the code, we have 3 thread run at the same time, so if we don't use mutex lock the thread running, it will lead to wrong result
  **Build and run**

```copy
make
```

```copy
./app
```

**Clean**

```copy
make clean
```

````
## Exercise 3
- create conditional variable
```bash
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
````

- wait for conditional variables

```bash
pthread_cond_wait(&cond_var, &mutex_lock);
```

- send signal cinditional variable

```bash
 pthread_cond_signal(&cond_var);
```

**Program explan**

- if buffer full, producer will stop make number
- if buffer empty, consumer will stop take number
- when begin consume thread, lock mutex, after finish will send sig to producer and release mutex
- when begin produce thread, lock mutex, after finish will send sig to consumer and release mutex
- Can only one thread run at the same time
  **Build and run**

```copy
make
```

```copy
./app
```

**Clean**

```copy
make clean
```

## Exercise 4

- create thread count odd number

```bash
pthread_create(&thread_odd,NULL , &count_odd_handler, NULL)
```

- create thread count event number

```bash
ret = pthread_create(&thread_even,NULL , &count_even_handler, NULL)
```

- wait for thread finish

```bash
pthread_join(thread_odd, NULL);
```

**Build and run**

```copy
make
```

```copy
./app
```

**Clean**

```copy
make clean
```

````
## Exercise 5
- create read/write lock
```bash
pthread_rwlock_init(&rwlock, NULL);
````

- all thread are allowed to read bu cannot write

```bash
pthread_rwlock_rdlock(&rwlock);
```

- only one thread can write at a time

```bash
 pthread_rwlock_wrlock(&rwlock);
```

- destroy read/write lock

```bash
pthread_rwlock_destroy(&rwlock);
```

**Code explanation**

- create thread read and write with ids and type
- make random threads
- run thread with random
- wait for thread finish
  **Build and run**

```copy
make
```

```copy
./app
```

**Clean**

```copy
make clean
```

## Exercise 6
**Code explanation**

- Create array with 1.000.000 random number
- Cal sum of one thread
- add result to g_sum
- cal sum of next thread and add to g_sum until and of 4 threads

**Build and run**

```copy
make
```

```copy
./app
```

**Clean**

```copy
make clean
```
