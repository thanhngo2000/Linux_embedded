#ifndef LOGGER_H
#define LOGGER_H
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "../../include/shared_data.h"

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/
typedef enum
{
    LOG_INFO,
    LOG_WARN,
    LOG_ERR
} LogLevel;
/******************************************************************************/
/*                            FUNCTIONS PROTOTYPES                             */
/******************************************************************************/
// init logger with mutex with AppContext
void logger_init(pthread_mutex_t *mutex);

// Print log with thread safe
void log_msg(LogLevel level, const char *fmt, ...);

#endif