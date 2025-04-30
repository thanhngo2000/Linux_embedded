// /******************************************************************************/
// /*                              INCLUDE FILES                                 */
// /******************************************************************************/

#include "../include/shared_data.h"
#include "utils/utils.h"
#include "storage/storage.h"
#include "logger/logger.h"
#include "data/data.h"
#include "connection/connection.h"
#include "user_interface/user_interface.h"
#include "security/security.h"

/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/
SystemManager system_manager;
volatile sig_atomic_t stop_requested = 0;
/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/

pthread_t connection_thread, storage_thread, data_thread;
pthread_t timeout_thread, update_thread;
pthread_t log_thread;

/******************************************************************************/
/*                            FUNCTIONS PROTOTYPES                             */
/******************************************************************************/
static void initialize_system(int port);
static void cleanup_system();
/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
/**
 * @brief Handle SIGINT signal (Ctrl+C)
 *
 * This function sets the `stop_requested` flag to 1, prints a cleanup message,
 * and handles the necessary cleanup steps before exiting the program.
 *
 * @param sig The signal number (unused here).
 */
static void handle_sigint(int sig)
{
    (void)sig;
    stop_requested = 1;
    write(STDOUT_FILENO, "\nSIGINT received. Cleaning up and exiting...\n", 45);
    // cleanup_system();
}
/**
 * @brief Register signal handlers
 *
 * This function registers the SIGINT signal handler to manage graceful program termination
 * when receiving SIGINT (Ctrl+C).
 */
static void register_signal_handlers()
{
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
}

/**
 * @brief Initialize system components
 *
 * This function initializes various system managers and creates threads for managing connections, storage, and data.
 *
 * @param port The port to be used by the connection manager.
 */
static void initialize_system(int port)
{
    init_log_manager();
    init_connection_manager();
    init_storage_manager();
    init_data_manager();
    init_ip_limiter_manager();
    init_ssl_context();

    pthread_create(&connection_thread, NULL, connection_manager, &port);
    // pthread_detach(connection_thread);

    pthread_create(&storage_thread, NULL, storage_manager, NULL);
    // pthread_detach(storage_thread);

    pthread_create(&data_thread, NULL, data_manager, NULL);
    // pthread_detach(data_thread);
}
/**
 * @brief Start additional background threads
 *
 * This function creates additional threads for tasks such as checking connection timeouts
 * and updating sensor data.
 */
static void start_background_threads()
{

    pthread_create(&timeout_thread, NULL, check_timeout_connection_thread, NULL);
    // pthread_detach(timeout_thread);

    pthread_create(&update_thread, NULL, update_sensor_data_thread, NULL);
    // pthread_detach(update_thread);
}
/**
 * @brief Cleanup all threads
 *
 * This function cancels and joins all threads created by the system.
 */
static void cleanup_threads()
{
    pthread_cancel(connection_thread);
    pthread_cancel(storage_thread);
    pthread_cancel(data_thread);
    // pthread_cancel(log_thread);
    pthread_cancel(timeout_thread);
    pthread_cancel(update_thread);

    pthread_join(connection_thread, NULL);
    pthread_join(storage_thread, NULL);
    pthread_join(data_thread, NULL);
    // pthread_join(log_thread, NULL);
    pthread_join(timeout_thread, NULL);
    pthread_join(update_thread, NULL);
}
/**
 * @brief Cleanup system resources
 *
 * This function calls the appropriate cleanup functions for system components such as
 * connection manager, data manager, storage manager, and SSL context.
 */
static void cleanup_system()
{
    cleanup_connection_manager();
    cleanup_data_manager();
    cleanup_storage_manager();
    cleanup_log_manager();
    cleanup_threads();
    cleanup_ssl_context();
}

/**
 * @brief Start the logging process in a separate process
 *
 * This function creates a child process using `fork()`. In the child process,
 * it starts the log manager in a new thread and waits for a signal.
 */
static void start_log_process()
{
    pid_t pid = fork();
    if (pid < 0)
    {
        handle_error("fork failed");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        // pthread_t log_thread;
        pthread_create(&log_thread, NULL, log_manager, NULL);
        // pthread_detach(log_thread);
        pthread_join(log_thread, NULL);
        pause(); // wait for signal
        exit(EXIT_SUCCESS);
    }
}

/**
 * @brief Set stdin to non-blocking mode
 *
 * This function sets the standard input (stdin) file descriptor to non-blocking
 * or blocking mode based on the `enable` parameter.
 *
 * @param enable If 1, set stdin to non-blocking mode, else set it to blocking mode.
 *
 * @return 0 on success, -1 on failure.
 */
static int set_stdin_nonblocking(int enable)
{
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1)
        return -1;

    if (enable)
        return fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    else
        return fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
}
/**
 * @brief Handle user input from stdin
 *
 * This function continuously waits for user input in non-blocking mode. It processes
 * commands when received and stops if a termination signal is received.
 */
static void handle_user_input()
{
    char user_input[BUFFER_SIZE];
    set_stdin_nonblocking(1);

    int prompt_shown = 0;

    while (!stop_requested)
    {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        struct timeval timeout = {1, 0};
        int ready = select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout);

        if (!prompt_shown)
        {
            printf("Enter your command: ");
            fflush(stdout);
            prompt_shown = 1;
        }

        if (ready == -1)
        {
            if (errno == EINTR)
                continue;
            perror("select");
            break;
        }
        else if (ready > 0 && FD_ISSET(STDIN_FILENO, &read_fds))
        {
            if (fgets(user_input, BUFFER_SIZE, stdin) != NULL)
            {
                user_input[strcspn(user_input, "\n")] = '\0'; // remove newline
                if (strlen(user_input) > 0)
                {
                    printf("Command received: %s\n", user_input);
                    handle_command(user_input);
                }
            }
            prompt_shown = 0;
        }
    }

    set_stdin_nonblocking(0);
}

// ==== Main Function ====

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);

    register_signal_handlers();
    start_background_threads();
    start_log_process();
    initialize_system(port);
    handle_user_input();
    cleanup_system();

    return EXIT_SUCCESS;
}
