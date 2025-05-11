
#ifndef UTILS_H
#define UTILS_H
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "../../include/shared_data.h"
#include "../logger/logger.h"
/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
char *get_ip();
void print_sync(AppContext *ctx, const char *format, ...);
bool is_peer_exists(PeerConnectionManager *manager, const char *ip, uint16_t port);
bool is_valid_ip_and_port(AppContext *ctx, const char *ip, uint16_t port);
#endif // UTILS_H
