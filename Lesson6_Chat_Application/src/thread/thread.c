
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "thread.h"
/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
/**
 * \brief Initializes the thread manager
 *
 * Sets initial thread count to 0 and initializes the mutex lock
 * \param tm Pointer to ThreadManager to initialize
 */
void thread_manager_init(ThreadManager *tm)
{
    tm->count = 0;
    pthread_mutex_init(&tm->lock, NULL);
}
/**
 * \brief Adds a thread to the thread manager
 *
 * Thread-safe operation to add a thread ID to the manager
 * \param tm Pointer to ThreadManager
 * \param tid Thread ID to add
 * \return true if added successfully, false if thread limit reached
 */
bool thread_manager_add(ThreadManager *tm, pthread_t tid)
{
    pthread_mutex_lock(&tm->lock);
    if (tm->count >= MAX_THREADS)
    {
        pthread_mutex_unlock(&tm->lock);
        return false;
    }
    tm->threads[tm->count++] = tid;
    pthread_mutex_unlock(&tm->lock);
    return true;
}
/**
 * \brief Joins all managed threads
 *
 * Waits for all threads in the manager to complete execution
 * \param tm Pointer to ThreadManager
 */
void thread_manager_join_all(ThreadManager *tm)
{
    pthread_mutex_lock(&tm->lock);
    for (int i = 0; i < tm->count; ++i)
    {
        pthread_join(tm->threads[i], NULL);
    }
    tm->count = 0;
    pthread_mutex_unlock(&tm->lock);
}
/**
 * \brief Destroys the thread manager
 *
 * Cleans up mutex resources, does not affect managed threads
 * \param tm Pointer to ThreadManager to destroy
 */
void thread_manager_destroy(ThreadManager *tm)
{
    pthread_mutex_destroy(&tm->lock);
}