
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "../include/shared_data.h"
#include "utils/utils.h"
#include "storage/storage.h"
#include "logger/logger.h"
#include "data/data.h"
#include "connection/connection.h"
#include "user_interface/user_interface.h"
#include "security/security.h"

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/

SystemManager system_manager;

/******************************************************************************/
/*                            FUNCTIONS PROTOTYPE                             */
/******************************************************************************/
void init_system_manager();
void cleanup_system_manager();
/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
/**
 * \brief Initializes all core managers required by the system.
 *
 * This function sets up the log manager, connection manager, storage manager,
 * data manager, and IP limiter manager to prepare the system for operation.
 *
 * \return void
 */
void init_system_manager()
{
    init_log_manager();
    init_connection_manager();
    init_storage_manager();
    init_data_manager();
    init_ip_limiter_manager();
}
/**
 * \brief Cleans up and releases all resources used by the system managers.
 *
 * This function deinitializes the connection, data, storage, and log managers
 * in the proper order to ensure safe shutdown of the system.
 *
 * \return void
 */
void cleanup_system_manager()
{
    cleanup_connection_manager();
    cleanup_data_manager();
    cleanup_storage_manager();
    cleanup_log_manager();
}
/**
 * \brief Starts a new detached thread with the specified function and arguments.
 *
 * \param thread Pointer to a pthread_t variable to store the created thread ID.
 * \param func Function pointer representing the entry point of the thread.
 * \param arg Pointer to the arguments passed to the thread function.
 * \param desc A descriptive name of the thread for logging or error messages.
 *
 * \return void
 */
static void start_thread(pthread_t *thread, void *(*func)(void *), void *arg, const char *desc)
{
    if (pthread_create(thread, NULL, func, arg) != 0)
    {
        fprintf(stderr, "Failed to create %s thread\n", desc);
        // exit(EXIT_FAILURE);
        return;
    }
    pthread_detach(*thread);
}
/**
 * \brief Main program
 *
 *
 * \param argc Argument count passed from the command line.
 * \param argv Array of command-line arguments.
 *
 * \return int Returns 0 on successful execution, or a non-zero value on failure.
 */
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // get local port
    int local_port = atoi(argv[1]);

    char user_input[BUFFER_SIZE];

    // create check timeout connection thread
    pthread_t timeout_thread;
    start_thread(&timeout_thread, check_timeout_connection_thread, NULL, "timeout");

    // create thread for update temp
    pthread_t temp_update_thread;
    start_thread(&temp_update_thread, sensor_data_update_thread, NULL, "sensor-update");

    // Create process
    pid_t pid = fork();
    if (pid < 0)
    {
        handle_error("fork failed");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {

        // Child process (log process)
        pthread_t log_thread;
        start_thread(&log_thread, log_manager, NULL, "log");
        pause();
    }
    else
    {
        init_system_manager();

        pthread_t connection_thread, storage_thread, data_thread;
        start_thread(&connection_thread, connection_manager, &local_port, "connection");
        start_thread(&storage_thread, storage_manager, NULL, "storage");
        start_thread(&data_thread, data_manager, NULL, "data");

        while (1)
        {
            sleep(1);
            printf("Enter your command: ");
            if (fgets(user_input, BUFFER_SIZE, stdin) == NULL)
            {
                handle_error("fgets failed");
            }
            printf("Command received: %s\n", user_input);

            // remove newline character
            user_input[strcspn(user_input, "\n")] = '\0';

            // handle command from user input function
            handle_command(user_input);
        }
    }

    // clean up when program end
    if (atexit(cleanup_system_manager) != 0)
    {
        perror("Failed to register cleanup function");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
