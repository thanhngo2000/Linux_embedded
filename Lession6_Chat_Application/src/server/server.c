/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "server.h"

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/

/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/

/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/
pthread_mutex_t lock;
/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
/**
 * @brief This function runs the server and listens for incoming connections.
 * It creates a new thread for each client connection to handle the communication.
 *
 * @param arg A pointer to the port number on which the server should listen.
 *            The port number is passed as an integer pointer.
 *
 * @return NULL pointer.
 */
void *run_server(void *arg)
{
    int server_sockfd, client_sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // create socket
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0)
    {
        handle_error("Socket creation failed");
    }

    peer.server_socket_fd = server_sockfd;

    // set ip address and port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(peer.port);

    // link the socket
    if (bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        handle_error("Bind failed");
    }

    // listen connection
    if (listen(server_sockfd, MAX_CONNECTIONS) < 0)
    {
        handle_error("Listen failed");
    }

    while (1)
    {
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_len);
        if (client_sockfd < 0)
        {
            handle_error("Socket Accept failed");
        }

        // create thread for client socket
        pthread_t recv_thread;
        if (pthread_create(&recv_thread, NULL, receive_messages, &client_sockfd) != 0)
        {
            handle_error("Failed to create receive thread");
        }
        pthread_detach(recv_thread);
    }
    close(server_sockfd);
    return NULL;
}

/**
 * @brief This function receives messages from a client and processes them.
 *
 * The function continuously receives messages from a client using the provided socket file descriptor.
 * Once a message is received, it is passed to the process_messages function for further processing.
 * The function runs in an infinite loop until the client closes the connection.
 *
 * @param arg A pointer to the socket file descriptor of the client.
 *            The socket file descriptor is passed as an integer pointer.
 *
 * @return NULL pointer.
 */
void *receive_messages(void *arg)
{
    int sockfd = *(int *)arg;
    char buffer[BUFFER_SIZE];

    while (1)
    {
        int bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0)
        {
            printf("Connection closed by peer in server.\n");
            close(sockfd);
            break;
        }
        buffer[bytes_received] = '\0';
        process_messages(sockfd, buffer);
    }
    return NULL;
}

/**
 * @brief Processes messages received from clients.
 *
 * This function handles different types of messages received from clients.
 * It checks the message type and performs the corresponding actions.
 *
 * @param sockfd The socket file descriptor of the client.
 * @param message The received message from the client.
 *
 * @return void
 */
void process_messages(const int sockfd, const char *message)
{
    // handle message
    if (strncmp(message, "connected", 9) == 0)
    {
        printf("Client connected to server\n");

        char words[MAX_WORDS][MAX_WORD_LENGTH];
        int num_strings = 3;

        split_string(message, words, num_strings);

        // printf("server received %d words from clients:\n", num_strings); // strings include connected command, ip, port
        // for (int i = 0; i < num_strings; i++)
        // {
        //     printf("%s\n", words[i]);
        // }

        add_connection(words[1], atoi(words[2]), sockfd);
    }
    else if (strncmp(message, "terminated", 10) == 0)
    {
        printf("Client terminated from server\n");

        // remove client from list
        // split message
        char words[MAX_WORDS][MAX_WORD_LENGTH];
        int num_info_terminated = 2;
        split_string(message, words, num_info_terminated);

        // for (int i = 0; i < num_info_terminated; i++)
        // {
        //     printf("%s\n", words[i]); // termianted <port>
        // }

        // get socket need to remove
        remove_connection(atoi(words[1]));
    }
    else if (strncmp(message, "send", 4) == 0)
    {
        printf("server receive message from client\n");

        // handle received message
        char words[MAX_WORDS][MAX_WORD_LENGTH];

        split_string(message, words, 4); // send <id> <port> <message>

        if (is_valid_ip(words[1]) && is_valid_port(atoi(words[1])))
        {
            printf("Message received from %s\n", words[1]);
            printf("Sender port: %d\n", atoi(words[2]));
            printf("Message: %s\n", words[3]);
        }
        else
        {
            handle_error("Failed: received invalid IP address or port");
        }
    }

    else if (strncmp(message, "exit", 4) == 0)
    {
        printf("Exit\n");
        char words[MAX_WORDS][MAX_WORD_LENGTH];

        split_string(message, words, 2); // exit <port>
        for (int i = 0; i < peer.active_connection_count; i++)
        {
            if (peer.active_connections[i].port == atoi(words[1]))
            {
                /* code */
                remove_connection(peer.active_connections[i].port);
            }
        }
    }
}
