#ifndef CONNECTION_H
#define CONNECTION_H
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "../../include/shared_data.h"
#include "../logger/logger.h"
/******************************************************************************/
/*                            FUNCTIONS PROTOTYPES                             */
/******************************************************************************/
PeerConnectionManager *create_peer_manager();
void init_peer_manager(PeerConnectionManager *manager);
void destroy_peer_manager(PeerConnectionManager *manager);
bool add_peer_manager(PeerConnectionManager *manager, const char *ip, uint16_t port, int sock_fd);
bool remove_peer_by_id(PeerConnectionManager *manager, int id);
bool remove_peer_by_socket(PeerConnectionManager *manager, int socket_fd);
void close_all_peer_manager(PeerConnectionManager *manager);
int count_peer_manager(PeerConnectionManager *manager);
PeerConnection *get_peer_by_index(PeerConnectionManager *manager, const int index);

#endif // CONNECTION_H