#ifndef CONNECTION_H
#define CONNECTION_H
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include <string.h>
#include <stdbool.h>

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

/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
void display_connections();

void add_connection(const char *ip_address, const int port, const int sockfd);
// void add_connection(const char *ip_address, const int port);
void remove_connection(const int port);
// int get_connection_socket(int port);

#endif // CONNECTION_H