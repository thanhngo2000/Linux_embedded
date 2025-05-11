/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "logger.h"
/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/
static pthread_mutex_t *logger_mutex = NULL;
/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
/**
 * \brief Initializes the logger system
 *
 * Sets the mutex to be used for thread synchronization in logging.
 * Must be called before any log operations.
 *
 * \param mutex Pointer to a pthread_mutex_t for thread safety
 */
void logger_init(pthread_mutex_t *mutex)
{
    logger_mutex = mutex;
}
/**
 * \brief Converts log level enum to string representation
 *
 * \param level LogLevel enum value to convert
 * \return Corresponding string representation ("INFO", "WARN", etc.)
 */
static const char *level_to_str(LogLevel level)
{
    switch (level)
    {
    case LOG_INFO:
        return "INFO";
    case LOG_WARN:
        return "WARN";
    case LOG_ERR:
        return "ERROR";
    default:
        return "LOG";
    }
}
/**
 * \brief Thread-safe logging function
 *
 * Formats and outputs log messages with timestamp and log level.
 * Ensures thread safety using mutex locking.
 *
 * \param level Log severity level (LOG_INFO, LOG_WARN, etc.)
 * \param fmt Format string for log message (printf-style)
 * \param ... Variable arguments for format string
 */
void log_msg(LogLevel level, const char *fmt, ...)
{
    if (!logger_mutex)
        return;

    pthread_mutex_lock(logger_mutex);

    // print log headline
    fprintf(stdout, "[%s] ", level_to_str(level));

    // print log content
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);

    fprintf(stdout, "\n");
    fflush(stdout);

    pthread_mutex_unlock(logger_mutex);
}