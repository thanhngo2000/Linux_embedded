#ifndef SOCKET_H
#define SOCKET_H
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "../../include/shared_data.h"
#include "../connection/connection.h"
#include "../message/message.h"
#include "../thread/thread.h"
#include "../logger/logger.h"

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/
typedef struct
{
    int client_sockfd;
    AppContext *ctx;
} ClientHandlerArgs;
/******************************************************************************/
/*                            FUNCTIONS PROTOTYPES                             */
/******************************************************************************/
void server_start(AppContext *ctx);
void client_start(const char *ip, const uint16_t port);
int establish_client_connection(const char *ip, uint16_t port);
void start_client_receive_thread(AppContext *ctx, int sockfd);
void socket_stop(AppContext *ctx);

#endif