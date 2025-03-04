/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/

#include "connection.h"

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
 * @brief Displays the list of active connections.
 *
 * This function acquires a lock on the mutex to ensure thread safety while accessing the shared data.
 * It then prints the header of the connection list and iterates through the active connections array to print each connection's details.
 * Finally, it releases the lock and prints a newline character.
 *
 * @return void
 */
void display_connections()
{
    pthread_mutex_lock(&lock); // lock mutex
    printf("\nConnected peers:\n");
    printf("ID\t\t\tIP Address\t\t\tPort\t\t\tSocketfd\n");
    // printf("ID\t\t\tIP Address\t\t\tPort\n");
    printf("------------------------------------------------------------\n");
    for (int i = 0; i < peer.active_connection_count; i++)
    {
        printf("%d\t\t\t%s\t\t\t%d\t\t\t%d\n", peer.active_connections[i].id,
               peer.active_connections[i].ip_address, peer.active_connections[i].port,
               peer.active_connections[i].socket_fd);
    }
    printf("\n");
    pthread_mutex_unlock(&lock);
}

/**
 * @brief Adds a new connection to the active connections list.
 *
 * This function adds a new connection to the active connections list by copying the provided IP address, port, and socket file descriptor.
 * It also increments the active connection count and assigns a unique ID to the new connection.
 * The function is thread-safe and acquires a lock on the mutex before accessing the shared data.
 *
 * @param ip_address The IP address of the new connection.
 * @param port The port number of the new connection.
 * @param sockfd The socket file descriptor of the new connection.
 *
 * @return void
 */
void add_connection(const char *ip_address, const int port, const int sockfd)
{
    pthread_mutex_lock(&lock); // lock mutex
    if (peer.active_connection_count < MAX_CONNECTIONS)
    {
        peer.active_connections[peer.active_connection_count].id = (peer.active_connection_count + 1);
        strncpy(peer.active_connections[peer.active_connection_count].ip_address, ip_address, INET_ADDRSTRLEN);
        peer.active_connections[peer.active_connection_count].port = port;
        peer.active_connections[peer.active_connection_count].socket_fd = sockfd;
        peer.active_connection_count++;
    }
    pthread_mutex_unlock(&lock); // unlock mutex
}

/**
 * @brief Removes a connection from the active connections list based on the given port.
 *
 * This function removes a connection from the active connections list by searching for a connection with the specified port.
 * If a connection is found, it shifts all subsequent connections up to fill the gap, effectively removing the connection.
 * The function is thread-safe and acquires a lock on the mutex before accessing the shared data.
 * After removing the connection, it calls the display_connections() function to print the updated list of active connections.
 *
 * @param port The port number of the connection to be removed.
 *
 * @return void
 */
void remove_connection(const int port)
{
    display_connections();
    pthread_mutex_lock(&lock); // lock mutex
    for (int i = 0; i < peer.active_connection_count; i++)
    {
        if (peer.active_connections[i].port == port)
        {
            // Shift subsequent connections up to fill the gap
            for (int j = i; j < peer.active_connection_count - 1; j++)
            {
                strncpy(peer.active_connections[j].ip_address, peer.active_connections[j + 1].ip_address, INET_ADDRSTRLEN);
                peer.active_connections[j].id = peer.active_connections[j + 1].id;
                peer.active_connections[j].port = peer.active_connections[j + 1].port;
                peer.active_connections[j].socket_fd = peer.active_connections[j + 1].socket_fd;
            }
            peer.active_connection_count--;
            break;
        }
    }

    pthread_mutex_unlock(&lock); // unlock mutex
    display_connections();
}
