#ifndef THREAD_H
#define THREAD_H
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "../../include/shared_data.h"
/******************************************************************************/
/*                            FUNCTIONS PROTOTYPES                             */
/******************************************************************************/
void thread_manager_init(ThreadManager *tm);
bool thread_manager_add(ThreadManager *tm, pthread_t tid);
void thread_manager_join_all(ThreadManager *tm);
void thread_manager_destroy(ThreadManager *tm);
#endif