/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "client.h"

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/

/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/

/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/

/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
/**
 * \brief This function establishes a connection to a server using a given IP address and port.
 *
 * \param ip_address A string representing the IP address of the server.
 * \param port An integer representing the port number of the server.
 *
 * \return void
 *
 */
void run_client(const char *ip_address, const int port)
{
    int sockfd;
    struct sockaddr_in target_addr;

    // create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        handle_error("Socket client creation failed");
    }
    printf("Socket socket created in client socket: %d\n", sockfd);

    int *sockfd_ptr = malloc(sizeof(int));
    *sockfd_ptr = sockfd;

    // set addr
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip_address, &target_addr.sin_addr) <= 0)
    {
        handle_error("Invalid IP address");
    }

    // connect server
    if (connect(sockfd, (struct sockaddr *)&target_addr, sizeof(target_addr)) < 0)
    {
        handle_error("Connection failed");
    }

    // save server info
    add_connection(ip_address, port, sockfd);
    // add_connection(ip_address, port);

    // send client info to server
    char message[BUFFER_SIZE];
    sprintf(message, "connected %s %d\n", peer.ip_address, peer.port);
    send(sockfd, message, strlen(message), 0);

    // create thread to receive message

    pthread_t recv_thread_client;
    if (pthread_create(&recv_thread_client, NULL, receive_messages_client, sockfd_ptr) != 0) // use ptr because sockfd is local address
    {
        handle_error("Failed to create receive thread");
        close(sockfd);
        free(sockfd_ptr);
        exit(EXIT_FAILURE);
    }
    pthread_detach(recv_thread_client);
}

/**
 * \brief This function is responsible for receiving messages from the server.
 *
 * This function is a thread function that continuously receives messages from the server using the provided socket file descriptor.
 * It handles the received messages by calling the process_messages_client function.
 *
 * \param arg A void pointer that points to the socket file descriptor.
 *
 * \return NULL upon successful execution.
 *
 */
void *receive_messages_client(void *arg)
{
    int sockfd = *(int *)arg;
    free(arg); // free socket ptr
    char buffer[BUFFER_SIZE];

    while (1)
    {
        int bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0)
        {
            printf("Connection closed by peer in client.\n");
            close(sockfd);
            break;
        }
        buffer[bytes_received] = '\0';
        process_messages_client(sockfd, buffer);
    }
    return NULL;
}

/**
 * \brief This function processes messages received from the server.
 *
 * The function handles different types of messages received from the server.
 * It checks the message type and performs the corresponding actions.
 *
 * \param sockfd The socket file descriptor used to communicate with the server.
 * \param message The received message from the server.
 *
 * \return void
 *
 */
void process_messages_client(const int sockfd, const char *message)
{
    // handle message
    if (strncmp(message, "connect", 7) == 0)
    {
        printf("Client connected to server\n");
    }
    else if (strncmp(message, "terminate", 9) == 0)
    {
        printf("Client terminated from server\n");

        // remove client from list
        // split message
        char words[MAX_WORDS][MAX_WORD_LENGTH];
        int num_info_terminated = 2;
        split_string(message, words, num_info_terminated);

        for (int i = 0; i < num_info_terminated; i++)
        {
            printf("%s\n", words[i]); // termianted <port>
        }

        // get socket need to remove
        remove_connection(atoi(words[1]));
    }
    else if (strncmp(message, "send", 4) == 0)
    {
        printf("Client receive message from server\n");

        // handle received message
        char receive_info[MAX_WORDS][MAX_WORD_LENGTH];

        split_string(message, receive_info, 4); // send <id> <port> <message>

        if (is_valid_ip(receive_info[1]) && is_valid_port(atoi(receive_info[1])))
        {
            printf("Message received from %s\n", receive_info[1]);
            printf("Sender port: %d\n", atoi(receive_info[2]));
            printf("Message: %s\n", receive_info[3]);
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
