
#include "logger.h"

/*---------------------------------------------------------*/
static void fifo_log(LogManager *self, LogLevel level, const char *source, const char *message)
{
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
            perror("Failed to write to FIFO");
            break;
        }

        total_written += bytes_written;
    }

    if (total_written < len)
    {
        fprintf(stderr, "Partial write to FIFO: expected %d, wrote %zd\n", len, total_written);
    }

    pthread_mutex_unlock(&impl->mutex);
}

static void fifo_destroy(LogManager *self)
{
    if (!self)
        return;
    FifoLogger *impl = (FifoLogger *)self->impl;
    if (impl)
    {
        close(impl->fifo_fd);
        pthread_mutex_destroy(&impl->mutex);
        free(impl);
    }
    free(self); // free malloc
}
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

void create_fifo_file(const char *fifo_file_name)
{
    // check fifo is exit
    if (access(fifo_file_name, F_OK) == -1)
    {
        // create fifo
        if (mkfifo(fifo_file_name, 0666) == -1)
        {
            perror("mkfifo failed");
            // return -1;
        }
        printf("FIFO created successfully\n");
    }
}

static LogManager *create_fifo_logger()
{

    FifoLogger *impl = malloc(sizeof(FifoLogger));
    if (!impl)
        return NULL;

    impl->fifo_fd = open(LOG_FIFO_NAME, O_WRONLY | O_NONBLOCK);
    if (impl->fifo_fd == -1)
    {
        perror("Failed to open FIFO for writing");
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

void init_log_manager()
{
    create_fifo_file(LOG_FIFO_NAME);
    create_log_file(LOG_FILE_NAME);
    usleep(100000);

    LogManager *logger = create_fifo_logger();

    system_manager.log_manager = *logger;

    FifoLogger fifo_logger;
    pthread_mutex_init(&fifo_logger.mutex, NULL);

    fifo_logger.log_count = 0;
}

void cleanup_log_manager()
{
    if (system_manager.log_manager.destroy)
    {
        system_manager.log_manager.destroy(&system_manager.log_manager);
    }
}
void read_log_file(const char *log_file_path)
{
    struct stat st;
    if (stat(log_file_path, &st) != 0)
    {
        fprintf(stderr, "Log file does not exist: %s\n", log_file_path);
        return;
    }

    FILE *fp = fopen(log_file_path, "r");
    if (!fp)
    {
        perror("Failed to open log file");
        return;
    }

    printf("=== Log file: %s ===\n", log_file_path);
    char line[512];
    while (fgets(line, sizeof(line), fp))
    {
        printf("%s", line);
    }

    fclose(fp);
    printf("=== End of log file ===\n");
}
int clear_log_file(const char *log_file_name)
{
    FILE *fp = fopen(log_file_name, "w");
    if (!fp)
    {
        perror("Failed to open log file for clearing");
        return -1;
    }

    // write log announce
    fprintf(fp, "=== Log file cleared at %ld ===\n", (long)time(NULL));

    fclose(fp);

    if (system_manager.log_manager.log)
    {
        system_manager.log_manager.log(&system_manager.log_manager,
                                       LOG_INFO,
                                       "System",
                                       "Log file has been cleared");
    }

    printf("Log file cleared successfully: %s\n", log_file_name);
    return 0;
}
void *log_manager(void *arg)
{
    // check fifo exit
    if (access(LOG_FIFO_NAME, F_OK) == -1)
    {
        if (mkfifo(LOG_FIFO_NAME, 0666) == -1)
        {
            perror("log_manager: mkfifo failed");
            return NULL;
        }
    }

    int fifo_fd = open(LOG_FIFO_NAME, O_RDONLY);
    if (fifo_fd == -1)
    {
        perror("log_manager: open FIFO failed");
        return NULL;
    }
    printf("Log manager: FIFO opened for reading\n");

    FILE *log_file;
    log_file = fopen(LOG_FILE_NAME, "a");
    if (!log_file)
    {
        perror("log_manager: open log file failed");
        close(fifo_fd);
        return NULL;
    }

    char buffer[512];
    while (1)
    {
        ssize_t count = read(fifo_fd, buffer, sizeof(buffer) - 1);
        if (count > 0)
        {
            buffer[count] = '\0';
            fprintf(log_file, "%s", buffer);
            fflush(log_file);
            fsync(fileno(log_file)); // sysn with storage
        }
        else if (count == 0)
        {
            // FIFO closed
            break;
        }
        else
        {
            perror("Error reading from FIFO");
            break;
        }
    }

    close(fifo_fd);
    fclose(log_file);
    return NULL;
}
