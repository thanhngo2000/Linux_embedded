/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include <pthread.h>
#include "user_interface/user_interface.h"
#include "server/server.h"
#include "client/client.h"
#include "../include/project_config.h"
#include "../include/client_server.h"
/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/

/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/

/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/
PeerInfo peer;
pthread_mutex_t lock;

/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // get local port
    peer.port = atoi(argv[1]);
    if (peer.port <= 0 || peer.port > 65535)
    {
        handle_error("Invalid port number. Please enter a port between 1 and 65535.");
        return EXIT_FAILURE;
    }

    // get local ip
    char *local_ip;
    local_ip = get_ip();
    strcpy(peer.ip_address, local_ip);

    // create mutex lock

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        handle_error("Mutex init failed");
    }

    print_menu();

    pthread_t server_thread;
    if (pthread_create(&server_thread, NULL, run_server, &peer.port) != 0)
    {
        handle_error("Failed to create server thread");
        return EXIT_FAILURE;
    }

    char user_input[BUFFER_SIZE];

    while (1)
    {
        printf("Enter your command: ");
        if (fgets(user_input, BUFFER_SIZE, stdin) == NULL)
        {
            handle_error("fgets failed");
        }
        printf("Command received: %s\n", user_input);

        // remove newline character
        user_input[strcspn(user_input, "\n")] = '\0';

        CommandHandler_handleCommand(&peer, user_input);
    }
    pthread_mutex_destroy(&lock);
    return EXIT_SUCCESS;
}