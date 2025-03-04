#ifndef CLIENT_SERVER_H
#define CLIENT_SERVER_H
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/
typedef struct
{
    int id;
    char ip_address[INET_ADDRSTRLEN];
    int port;
    int socket_fd;
} ConnectionInfo;
typedef struct
{
    char ip_address[INET_ADDRSTRLEN];                   // IP peer
    int port;                                           // port peer
    int server_socket_fd;                               // socket fd peer
    ConnectionInfo active_connections[MAX_CONNECTIONS]; // list peer
    int active_connection_count;                        // num active connections
} PeerInfo;

extern PeerInfo peer;

extern pthread_mutex_t lock;
/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/

/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/

/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/

#endif // CLIENT_SERVER_H