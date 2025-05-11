/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "socket.h"
/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/
extern volatile sig_atomic_t keep_running;
/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/
static void *server_accept_loop(void *arg);
static void *client_receive_loop(void *arg);
static int create_server_socket(uint16_t port);

/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
/**
 * \brief Starts the chat server
 *
 * Creates server socket and launches server accept thread
 * \param ctx Application context containing server configuration
 */
void server_start(AppContext *ctx)
{
    int sockfd = create_server_socket(ctx->local_info.port);
    if (sockfd < 0)
    {
        log_msg(LOG_ERR, "socket_start: Failed to create server socket: %s", strerror(errno));
        return;
    }

    ctx->local_info.socket_fd = sockfd;

    pthread_t server_thread;
    if (pthread_create(&server_thread, NULL, server_accept_loop, ctx) != 0)
    {
        log_msg(LOG_ERR, "socket_start: Failed to create server thread: %s", strerror(errno));
        close(sockfd);
        return;
    }
    thread_manager_add(&ctx->thread_manager, server_thread);
}
/**
 * \brief Stops the chat server and cleans up resources
 *
 * Signals threads to stop, closes sockets and peer connections
 * \param ctx Application context
 */
void socket_stop(AppContext *ctx)
{
    keep_running = false;

    shutdown(ctx->local_info.socket_fd, SHUT_RDWR);
    close(ctx->local_info.socket_fd);

    close_all_peer_manager(ctx->peer_manager);
}
/**
 * \brief Creates and configures a TCP server socket
 *
 * Sets up socket with SO_REUSEADDR and binds to specified port
 * \param port Port number to listen on
 * \return Socket file descriptor or -1 on error
 */
static int create_server_socket(uint16_t port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        return -1;

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, MAX_PEERS) < 0)
    {
        close(sockfd);
        return -1;
    }

    return sockfd;
}
/**
 * \brief Main server accept loop
 *
 * Accepts incoming client connections and spawns handler threads
 * \param arg AppContext pointer cast from thread argument
 * \return NULL when thread exits
 */
static void *server_accept_loop(void *arg)
{
    AppContext *ctx = (AppContext *)arg;
    int server_sockfd = ctx->local_info.socket_fd;

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (keep_running)
    {
        int client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_len);
        if (client_sockfd < 0)
        {
            if (!keep_running)
                break; // shutdown() triggered accept() failure
            log_msg(LOG_ERR, "server_accept_loop: Accept failed: %s", strerror(errno));
            continue;
        }

        pthread_t recv_client_thread;
        ClientHandlerArgs *args = malloc(sizeof(ClientHandlerArgs));
        if (!args)
        {
            log_msg(LOG_ERR, "server_accept_loop: malloc failed: %s", strerror(errno));
            close(client_sockfd);
            remove_peer_by_socket(ctx->peer_manager, client_sockfd);
            continue;
        }

        args->client_sockfd = client_sockfd;
        args->ctx = ctx;

        if (pthread_create(&recv_client_thread, NULL, client_receive_loop, args) != 0)
        {
            log_msg(LOG_ERR, "server_accept_loop: Failed to create client thread: %s", strerror(errno));
            close(client_sockfd);
            remove_peer_by_socket(ctx->peer_manager, client_sockfd);
            free(args);
        }
        else
        {
            thread_manager_add(&ctx->thread_manager, recv_client_thread);
        }
    }

    log_msg(LOG_INFO, "server_accept_loop: exiting");
    return NULL;
}
/**
 * \brief Client message receive loop
 *
 * Handles incoming messages from a connected client with receive timeout
 * \param arg ClientHandlerArgs pointer cast from thread argument
 * \return NULL when thread exits
 */
static void *client_receive_loop(void *arg)
{
    ClientHandlerArgs *args = (ClientHandlerArgs *)arg;
    int sockfd = args->client_sockfd;
    AppContext *ctx = args->ctx;
    free(args);

    // ⚠️ Set recv timeout for avoiding permanent blocking
    struct timeval timeout = {.tv_sec = 1, .tv_usec = 0};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    char buffer[MAX_MESSAGE_LENGTH];

    while (keep_running)
    {
        int received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (received < 0)
        {
            continue; // timeout
        }
        else if (received == 0)
        {
            log_msg(LOG_INFO, "client_receive_loop: Peer disconnected (fd=%d)", sockfd);
            break;
        }

        buffer[received] = '\0';
        handle_message_receive(ctx, sockfd, buffer);
    }

    close(sockfd);
    remove_peer_by_socket(ctx->peer_manager, sockfd);
    log_msg(LOG_INFO, "client_receive_loop: exiting (fd=%d)", sockfd);
    return NULL;
}
/**
 * \brief Establishes client connection to remote server
 *
 * Creates socket and connects to specified IP/port
 * \param ip Server IP address to connect to
 * \param port Server port number
 * \return Socket file descriptor on success, -1 on failure
 */
int establish_client_connection(const char *ip, uint16_t port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        log_msg(LOG_ERR, "socket(): %s", strerror(errno));
        return -1;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
    {
        log_msg(LOG_ERR, "inet_pton(): %s", strerror(errno));
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        log_msg(LOG_ERR, "connect(): %s", strerror(errno));
        close(sockfd);
        return -1;
    }

    return sockfd;
}
/**
 * \brief Starts client receive thread
 *
 * Creates thread to handle incoming messages from server
 * \param ctx Application context
 * \param sockfd Socket file descriptor for the connection
 */
void start_client_receive_thread(AppContext *ctx, int sockfd)
{
    pthread_t recv_server_thread;
    ClientHandlerArgs *args = malloc(sizeof(ClientHandlerArgs));
    if (!args)
    {
        log_msg(LOG_ERR, "start_receive_thread: malloc failed: %s", strerror(errno));
        return;
    }

    args->client_sockfd = sockfd;
    args->ctx = ctx;

    if (pthread_create(&recv_server_thread, NULL, client_receive_loop, args) != 0)
    {
        log_msg(LOG_ERR, "start_receive_thread: Failed to create receive thread: %s", strerror(errno));
        free(args);
        return;
    }

    thread_manager_add(&ctx->thread_manager, recv_server_thread);
}