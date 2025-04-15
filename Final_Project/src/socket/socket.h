#ifndef SOCKET_H
#define SOCKET_H
#include "../../include/shared_data.h"
#include "../utils/utils.h"
#include "../security/security.h"

typedef struct
{
    char *ip_address;
    int port;
    int local_port;
} ConnectArgs;

typedef struct
{
    int sock_fd;
    char ip_address[INET_ADDRSTRLEN];
    int port;
} ClientInfoPacket;

/*Function prototype*/

int setup_epoll(int server_fd);
void handle_new_connection(int epoll_fd, int server_fd);
int create_and_bind_socket(const int port);
void *client_thread_main(void *arg);
void start_client_connection(char *ip_address, const int port_connect);
int accept_client(int server_fd, struct sockaddr_in *client_addr, socklen_t *client_len);
bool read_client_info(int client_fd, ClientInfoPacket *packet);
void warn_if_ip_mismatch(const char *actual_ip, const char *reported_ip);
bool add_client_fd_to_epoll(int epoll_fd, int client_fd);

#endif