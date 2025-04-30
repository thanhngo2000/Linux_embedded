
#include "logger.h"

/**
 * \brief Logs a message to a FIFO log with the specified log level, source, and message.
 *
 * \param self Pointer to the LogManager instance.
 * \param level The severity level of the log (e.g., INFO, WARNING, ERROR).
 * \param source The source of the log message (e.g., "Data", "System").
 * \param message The message to log.
 *
 * \note This function writes the log message to a FIFO with thread safety.
 */
static void fifo_log(LogManager *self, LogLevel level, const char *source, const char *message)
{
    if (!self || !self->impl)
        return;

    FifoLogger *impl = (FifoLogger *)self->impl;

    time_t now = time(NULL);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

    char formatted_msg[512];
    int len = snprintf(formatted_msg, sizeof(formatted_msg), "%d|%s|%s|%s\n",
                       impl->log_count++, time_str, source, message);

    pthread_mutex_lock(&impl->mutex);

    ssize_t total_written = 0;
    while (total_written < len)
    {
        ssize_t bytes_written = write(impl->fifo_fd, formatted_msg + total_written, len - total_written);
        if (bytes_written == -1)
        {
            perror("fifo_log: write error");
            break;
        }
        total_written += bytes_written;
    }

    if (total_written < len)
    {
        fprintf(stderr, "fifo_log: partial write to FIFO (expected %d, wrote %zd)\n", len, total_written);
    }

    pthread_mutex_unlock(&impl->mutex);
}
/**
 * \brief Cleans up the resources used by the FIFO logger, including closing the FIFO and freeing memory.
 *
 * \param self Pointer to the LogManager instance to clean up.
 *
 * \return void
 */
static void fifo_destroy(LogManager *self)
{
    if (!self || !self->impl)
        return;

    FifoLogger *impl = (FifoLogger *)self->impl;
    close(impl->fifo_fd);
    pthread_mutex_destroy(&impl->mutex);
    free(impl);
    self->impl = NULL;
}
/**
 * \brief Creates a log file if it does not already exist.
 *
 * \param log_file_name The name of the log file to create.
 *
 * \return void
 *
 * \note If the log file already exists, this function simply prints a message.
 */
void create_log_file(const char *log_file_name)
{
    struct stat st;
    if (stat(log_file_name, &st) != 0)
    {
        // file if not exit, create new
        FILE *fp = fopen(log_file_name, "w");
        if (!fp)
        {
            perror("Failed to create log file");
            return;
        }
        fprintf(fp, "=== Log file created ===\n");
        fclose(fp);
        printf("Log file created: %s\n", log_file_name);
    }
    else
    {
        // file exit
        printf("Log file already exists: %s\n", log_file_name);
    }
}
/**
 * \brief Creates a FIFO file if it does not already exist.
 *
 * \param fifo_file_name The name of the FIFO file to create.
 *
 * \return void
 *
 * \note If the FIFO file already exists, this function prints a message.
 */

void create_fifo_file(const char *fifo_name)
{
    if (access(fifo_name, F_OK) == -1)
    {
        if (mkfifo(fifo_name, 0666) == -1)
        {
            perror("create_fifo_file: mkfifo");
        }
    }
}
/**
 * \brief Creates a FIFO logger that writes logs to a FIFO file.
 *
 * \return LogManager* A pointer to the created LogManager instance, or NULL on failure.
 *
 * \note The LogManager structure is initialized with a FIFO logger implementation that writes to a FIFO.
 */
static LogManager *create_fifo_logger()
{
    FifoLogger *impl = malloc(sizeof(FifoLogger));
    if (!impl)
        return NULL;

    int retry = 5;
    while ((impl->fifo_fd = open(LOG_FIFO_NAME, O_WRONLY | O_NONBLOCK)) == -1 && retry-- > 0)
    {
        perror("create_fifo_logger: waiting for FIFO reader...");
        usleep(100000); // 100ms
    }

    if (impl->fifo_fd == -1)
    {
        perror("create_fifo_logger: failed to open FIFO for writing");
        free(impl);
        return NULL;
    }

    pthread_mutex_init(&impl->mutex, NULL);
    impl->log_count = 0;

    LogManager *logger = malloc(sizeof(LogManager));
    if (!logger)
    {
        close(impl->fifo_fd);
        pthread_mutex_destroy(&impl->mutex);
        free(impl);
        return NULL;
    }

    logger->log = fifo_log;
    logger->destroy = fifo_destroy;
    logger->impl = impl;

    return logger;
}
/**
 * \brief Initializes the log manager by creating necessary files and setting up the FIFO logger.
 *
 * \return void
 *
 * \note This function creates the FIFO file and log file if they do not exist and sets up the log manager.
 */
void init_log_manager()
{
    create_fifo_file(LOG_FIFO_NAME);
    create_log_file(LOG_FILE_NAME);
    usleep(100000); // Chờ FIFO reader

    LogManager *logger_ptr = create_fifo_logger();
    if (!logger_ptr)
    {
        fprintf(stderr, "init_log_manager: Failed to initialize logger\n");
        return;
    }

    system_manager.log_manager = *logger_ptr;
    free(logger_ptr);
}
/**
 * \brief Cleans up the resources used by the log manager.
 *
 * \return void
 *
 * \note This function calls the destroy method on the LogManager to release resources.
 */
void cleanup_log_manager()
{
    if (system_manager.log_manager.destroy)
    {
        system_manager.log_manager.destroy(&system_manager.log_manager);
    }
}
/**
 * \brief Reads and prints the content of the specified log file.
 *
 * \param log_file_path The path to the log file to read.
 *
 * \return void
 *
 * \note This function reads the log file line by line and prints each line to the standard output.
 */
void read_log_file(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (!fp)
    {
        perror("read_log_file: fopen");
        return;
    }

    printf("=== Log File (%s) ===\n", path);
    char line[512];
    while (fgets(line, sizeof(line), fp))
    {
        printf("%s", line);
    }
    fclose(fp);
    printf("=== End of Log ===\n");
}
/**
 * \brief Clears the contents of the specified log file and logs an informational message.
 *
 * \param log_file_name The name of the log file to clear.
 *
 * \return int Returns 0 if successful, or -1 on error.
 *
 * \note This function overwrites the log file with a clearing message and logs an informational message.
 */
int clear_log_file(const char *log_file_name)
{
    FILE *fp = fopen(log_file_name, "w");
    if (!fp)
    {
        perror("clear_log_file: fopen");
        return -1;
    }

    fprintf(fp, "=== Log file cleared at %ld ===\n", (long)time(NULL));
    fclose(fp);

    if (system_manager.log_manager.log)
    {
        system_manager.log_manager.log(&system_manager.log_manager, LOG_INFO,
                                       "System", "Log file has been cleared");
    }

    return 0;
}

/**
 * \brief The main function for the log manager thread that reads logs from a FIFO and writes them to a log file.
 *
 * \param arg A pointer to any arguments passed to the thread (not used here).
 *
 * \return void* Always returns NULL.
 *
 * \note This function continuously reads messages from a FIFO and writes them to a log file.
 * It ensures proper synchronization and handles errors in reading from the FIFO.
 */
void *log_manager(void *arg)
{
    create_fifo_file(LOG_FIFO_NAME);

    int fifo_fd = open(LOG_FIFO_NAME, O_RDONLY);
    if (fifo_fd == -1)
    {
        perror("log_manager: open FIFO");
        return NULL;
    }

    FILE *log_file = fopen(LOG_FILE_NAME, "a");
    if (!log_file)
    {
        perror("log_manager: fopen log file");
        close(fifo_fd);
        return NULL;
    }

    char buffer[512];
    while (!stop_requested)
    {
        ssize_t count = read(fifo_fd, buffer, sizeof(buffer) - 1);
        if (count > 0)
        {
            buffer[count] = '\0';
            fprintf(log_file, "%s", buffer);
            fflush(log_file);
            fsync(fileno(log_file));
        }
        else if (count == 0)
        {
            break; // FIFO closed
        }
        else
        {
            perror("log_manager: read error");
            break;
        }
    }

    close(fifo_fd);
    fclose(log_file);
    return NULL;
}
