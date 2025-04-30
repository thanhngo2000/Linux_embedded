#ifndef UTILS_H
#define UTILS_H
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "../../include/shared_data.h"
/******************************************************************************/
/*                            FUNCTIONS PROTOTYPES                             */
/******************************************************************************/
void handle_error(const char *message);
bool is_valid_ip(const char *ip);
bool is_valid_port(const int port);
bool is_running_port(const int port, const int running_port);
bool is_port_already_connected(const int port);
int validate_connection_params(char *ip, const int port, const int running_port);
#endif // UTILS_H
