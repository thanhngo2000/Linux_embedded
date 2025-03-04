#ifndef SERVER_H
#define SERVER_H
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "../utils/utils.h"
#include "../connection/connection.h"
#include "../include/project_config.h"
#include "../include/client_server.h"

/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/

/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/

/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
void *run_server(void *arg);
void *receive_messages(void *arg);
void process_messages(const int sockfd, const char *message);

#endif // SERVER_H