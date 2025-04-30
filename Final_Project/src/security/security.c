/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "security.h"
/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
/**
 * \brief Sends data over an SSL connection.
 *
 * This function wraps the SSL_write function to send data over an SSL
 * connection. It handles SSL errors and prints them if the operation fails.
 *
 * \param self Pointer to the SSLConnection instance.
 * \param data Data to be sent over the connection.
 * \param len Length of the data to send.
 *
 * \return The number of bytes written on success, -1 on failure.
 */
static int ssl_send(void *self, const char *data, size_t len)
{
    SSLConnection *conn = (SSLConnection *)self;
    int ret = SSL_write(conn->ssl, data, len);
    if (ret <= 0)
    {
        int err = SSL_get_error(conn->ssl, ret);
        fprintf(stderr, "SSL_write failed: %d\n", err);
        ERR_print_errors_fp(stderr);
        return -1;
    }
    return ret;
}
/**
 * \brief Receives data from an SSL connection.
 *
 * This function wraps the SSL_read function to receive data over an SSL
 * connection. It handles SSL errors and prints them if the operation fails.
 *
 * \param self Pointer to the SSLConnection instance.
 * \param buffer Buffer to store the received data.
 * \param len Maximum length of data to receive.
 *
 * \return The number of bytes read on success, -1 on failure.
 */
static int ssl_recv(void *self, char *buffer, size_t len)
{
    SSLConnection *conn = (SSLConnection *)self;
    int ret = SSL_read(conn->ssl, buffer, len);
    if (ret <= 0)
    {
        int err = SSL_get_error(conn->ssl, ret);
        fprintf(stderr, "SSL_read failed: %d\n", err);
        ERR_print_errors_fp(stderr);
        return -1;
    }
    return ret;
}
/**
 * \brief Closes an SSL connection and frees associated resources.
 *
 * This function ensures proper shutdown of the SSL connection by calling
 * SSL_shutdown, and frees all allocated resources related to the connection.
 *
 * \param impl Pointer to the SSLConnection instance.
 *
 * \return void
 */
void ssl_close(void *impl)
{
    SSLConnection *ssl_conn = (SSLConnection *)impl;
    if (ssl_conn)
    {
        // call SSL_shutdown make sure SSL close
        int shutdown_ret = SSL_shutdown(ssl_conn->ssl);
        if (shutdown_ret == 0)
        {
            SSL_shutdown(ssl_conn->ssl);
        }
        else if (shutdown_ret < 0)
        {
            int err = SSL_get_error(ssl_conn->ssl, shutdown_ret);
            fprintf(stderr, "SSL_shutdown failed: %d\n", err);
            ERR_print_errors_fp(stderr);
        }

        SSL_free(ssl_conn->ssl); // free ssl
        close(ssl_conn->fd);     // close
        free(ssl_conn);
    }
}
/**
 * \brief Sends data over a plain (non-SSL) connection.
 *
 * This function sends data over a plain TCP connection using the write system call.
 * It handles errors related to the write operation.
 *
 * \param self Pointer to the PlainConnection instance.
 * \param data Data to be sent.
 * \param len Length of the data.
 *
 * \return The number of bytes written on success, -1 on failure.
 */
static int plain_send(void *self, const char *data, size_t len)
{
    PlainConnection *conn = (PlainConnection *)self;
    ssize_t ret = write(conn->fd, data, len);
    if (ret < 0)
    {
        perror("write failed");
        return -1;
    }
    return ret;
}
/**
 * \brief Receives data from a plain (non-SSL) connection.
 *
 * This function receives data over a plain TCP connection using the read system call.
 * It handles errors related to the read operation.
 *
 * \param self Pointer to the PlainConnection instance.
 * \param buffer Buffer to store received data.
 * \param len Maximum length of data to receive.
 *
 * \return The number of bytes read on success, -1 on failure.
 */
static int plain_recv(void *self, char *buffer, size_t len)
{
    PlainConnection *conn = (PlainConnection *)self;
    ssize_t ret = read(conn->fd, buffer, len);
    if (ret < 0)
    {
        perror("read failed");
        return -1;
    }
    return ret;
}
/**
 * \brief Closes a plain (non-SSL) connection and frees associated resources.
 *
 * This function closes a plain TCP connection and frees resources associated with it.
 *
 * \param impl Pointer to the PlainConnection instance.
 *
 * \return void
 */
static void plain_close(void *impl)
{
    PlainConnection *conn = (PlainConnection *)impl;
    if (conn)
    {
        close(conn->fd);
        free(conn);
    }
}
/**
 * \brief Creates an SSL server connection.
 *
 * This function initializes a new SSL connection on the server side, performing
 * the SSL handshake using SSL_accept. It returns a pointer to the SSLConnection
 * instance on success, or NULL if the handshake fails.
 *
 * \param fd The file descriptor for the server socket.
 * \param ctx The SSL context for the server.
 *
 * \return Pointer to an SSLConnection instance or NULL on failure.
 */
static SSLConnection *ssl_connection_create_server(int fd, SSL_CTX *ctx)
{
    SSLConnection *conn = malloc(sizeof(SSLConnection));
    if (!conn)
        return NULL;

    conn->ssl = SSL_new(ctx);
    conn->ctx = ctx;
    conn->fd = fd;

    SSL_set_fd(conn->ssl, fd);
    printf("[SSL SERVER] Waiting for SSL_accept...\n");

    int ret = SSL_accept(conn->ssl);
    if (ret <= 0)
    {
        int err = SSL_get_error(conn->ssl, ret);
        fprintf(stderr, "SSL_accept failed: %d\n", err);
        ERR_print_errors_fp(stderr); // In chi tiết lỗi SSL
        SSL_free(conn->ssl);
        free(conn);
        return NULL;
    }

    printf("[SSL SERVER] Handshake success\n");
    return conn;
}
/**
 * \brief Creates an SSL client connection.
 *
 * This function initializes a new SSL connection on the client side, performing
 * the SSL handshake using SSL_connect. It returns a pointer to the SSLConnection
 * instance on success, or NULL if the handshake fails.
 *
 * \param fd The file descriptor for the client socket.
 * \param ctx The SSL context for the client.
 *
 * \return Pointer to an SSLConnection instance or NULL on failure.
 */
static SSLConnection *ssl_connection_create_client(int fd, SSL_CTX *ctx)
{
    SSLConnection *conn = malloc(sizeof(SSLConnection));
    if (!conn)
        return NULL;

    conn->ssl = SSL_new(ctx);
    conn->ctx = ctx;
    conn->fd = fd;

    SSL_set_fd(conn->ssl, fd);
    printf("[SSL CLIENT] Attempting SSL_connect...\n");

    int ret = SSL_connect(conn->ssl);
    if (ret <= 0)
    {
        int err = SSL_get_error(conn->ssl, ret);
        fprintf(stderr, "SSL_connect failed: %d\n", err);
        ERR_print_errors_fp(stderr); // In chi tiết lỗi SSL
        SSL_free(conn->ssl);
        free(conn);
        return NULL;
    }

    printf("[SSL CLIENT] Handshake success\n");
    return conn;
}

/**
 * \brief Destroys a secure connection and frees its resources.
 *
 * This function closes the connection, calls the appropriate close method for
 * the secure communication type, and frees associated memory.
 *
 * \param comm Pointer to the SecureCommunication instance.
 *
 * \return void
 */
void destroy_secure_connection(SecureCommunication *comm)
{
    if (!comm)
        return;
    if (comm->interface.close)
        comm->interface.close(comm->impl);
    free(comm);
}
/**
 * \brief Creates a secure connection (SSL or plain).
 *
 * This function creates a secure communication instance based on the specified mode (SSL or plain).
 * It returns a pointer to the SecureCommunication instance or NULL on failure.
 *
 * \param fd The file descriptor for the connection.
 * \param mode The security mode (SSL or plain).
 *
 * \return Pointer to a SecureCommunication instance or NULL on failure.
 */
SecureCommunication *create_secure_connection(int fd, SecureMode mode)
{
    SecureCommunication *comm = malloc(sizeof(SecureCommunication));
    if (!comm)
        return NULL;

    switch (mode)
    {
    case SECURE_SSL_SERVER:
        comm->impl = ssl_connection_create_server(fd, system_manager.ssl_server_context);
        break;

    case SECURE_SSL_CLIENT:
        comm->impl = ssl_connection_create_client(fd, system_manager.ssl_client_context);
        break;

    case SECURE_PLAIN:
        comm->impl = malloc(sizeof(PlainConnection));
        if (comm->impl)
            ((PlainConnection *)comm->impl)->fd = fd;
        break;
    }

    if (!comm->impl)
    {
        free(comm);
        return NULL;
    }

    comm->interface.send = (mode == SECURE_PLAIN) ? plain_send : ssl_send;
    comm->interface.recv = (mode == SECURE_PLAIN) ? plain_recv : ssl_recv;
    comm->interface.close = (mode == SECURE_PLAIN) ? plain_close : ssl_close;

    return comm;
}
/**
 * \brief Initializes the SSL contexts for both server and client.
 *
 * This function initializes the SSL context for both server and client, loading
 * the necessary certificates and keys for server-side SSL connections, and
 * configuring the client context with certificate verification.
 *
 * \return void
 */
void init_ssl_context()
{
    // ─── SERVER CONTEXT ─────────────────────────────────────────────
    const SSL_METHOD *server_method = TLS_server_method();
    system_manager.ssl_server_context = SSL_CTX_new(server_method);
    if (!system_manager.ssl_server_context)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_certificate_file(system_manager.ssl_server_context, "./cert.pem", SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(system_manager.ssl_server_context, "./cert.key", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (!SSL_CTX_check_private_key(system_manager.ssl_server_context))
    {
        fprintf(stderr, "Private key mismatch.\n");
        exit(EXIT_FAILURE);
    }

    // ─── CLIENT CONTEXT ─────────────────────────────────────────────
    const SSL_METHOD *client_method = TLS_client_method();
    system_manager.ssl_client_context = SSL_CTX_new(client_method);
    if (!system_manager.ssl_client_context)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Optional: enable certificate verification from server
    SSL_CTX_set_verify(system_manager.ssl_client_context, SSL_VERIFY_PEER, NULL);
    if (SSL_CTX_load_verify_locations(system_manager.ssl_client_context, "./cert.pem", NULL) <= 0)
    {
        ERR_print_errors_fp(stderr);
    }
}
/**
 * \brief Cleans up and frees the SSL contexts.
 *
 * This function frees the SSL context resources for both the server and client.
 *
 * \return void
 */
void cleanup_ssl_context()
{
    if (system_manager.ssl_server_context)
    {
        SSL_CTX_free(system_manager.ssl_server_context); // Giải phóng SSL_CTX của server
        system_manager.ssl_server_context = NULL;
    }

    if (system_manager.ssl_client_context)
    {
        SSL_CTX_free(system_manager.ssl_client_context); // Giải phóng SSL_CTX của client
        system_manager.ssl_client_context = NULL;
    }
}
/*-----------Protect DoS attack---------------------------------------------*/
/**
 * \brief Returns a pointer to the IP limiter manager.
 *
 * This function provides access to the IP limiter manager used for managing
 * IP connection limits to prevent DoS attacks.
 *
 * \return Pointer to the IpLimiterManager instance.
 */
static IpLimiterManager *get_ip_limiter_manager()
{
    return &system_manager.ip_limiter_manager;
}
/**
 * \brief Initializes the IP limiter manager.
 *
 * This function initializes the IP limiter manager, setting up the mutex
 * and clearing any stored IP entries.
 *
 * \return void
 */
void init_ip_limiter_manager()
{

    IpLimiterManager *ip_manager = get_ip_limiter_manager();

    ip_manager->size = 0;

    pthread_mutex_init(&ip_manager->mutex, NULL);

    // create IP in list
    for (int i = 0; i < MAX_UNIQUE_IPS; ++i)
    {
        ip_manager->entries[i].ip[0] = '\0';
        ip_manager->entries[i].count = 0;
    }
}
/**
 * \brief Checks if an IP address is allowed to establish a connection.
 *
 * This function checks if the number of connections from a specific IP
 * address has exceeded the allowed limit. If so, the connection is denied.
 *
 * \param ip The IP address to check.
 *
 * \return true if the IP is allowed to connect, false if it is not.
 */
bool ip_limiter_allow_connection(const char *ip)
{
    IpLimiterManager *ip_manager = get_ip_limiter_manager();

    pthread_mutex_lock(&ip_manager->mutex);

    for (int i = 0; i < ip_manager->size; ++i)
    {
        if (strcmp(ip_manager->entries[i].ip, ip) == 0)
        {
            if (ip_manager->entries[i].count >= MAX_CONNECTIONS_PER_IP)
            {
                pthread_mutex_unlock(&ip_manager->mutex);
                return false;
            }
            ip_manager->entries[i].count++;
            pthread_mutex_unlock(&ip_manager->mutex);
            return true;
        }
    }

    if (ip_manager->size < MAX_UNIQUE_IPS)
    {
        strncpy(ip_manager->entries[ip_manager->size].ip, ip, sizeof(ip_manager->entries[ip_manager->size].ip) - 1);
        ip_manager->entries[ip_manager->size].count = 1;
        ip_manager->size++;
        pthread_mutex_unlock(&ip_manager->mutex);
        return true;
    }

    pthread_mutex_unlock(&ip_manager->mutex);
    return false; // out of size for store IP
}
/**
 * \brief Removes a connection from the IP limiter's tracking.
 *
 * This function decreases the connection count for a given IP address.
 * If the count is greater than zero, it decrements it.
 *
 * \param ip The IP address to remove from the limiter.
 *
 * \return void
 */
void ip_limiter_remove_connection(const char *ip)
{
    IpLimiterManager *ip_manager = get_ip_limiter_manager();

    pthread_mutex_lock(&ip_manager->mutex);

    for (int i = 0; i < ip_manager->size; ++i)
    {
        if (strcmp(ip_manager->entries[i].ip, ip) == 0 && ip_manager->entries[i].count > 0)
        {
            ip_manager->entries[i].count--;
            pthread_mutex_unlock(&ip_manager->mutex);
            return;
        }
    }

    pthread_mutex_unlock(&ip_manager->mutex);
}
