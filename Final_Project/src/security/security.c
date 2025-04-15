#include "security.h"

/*-----------------------SSL wrapper------------------------*/

SSLWrapper *ssl_wrapper_create_server(int fd, SSL_CTX *ctx)
{
    SSLWrapper *wrapper = malloc(sizeof(SSLWrapper));
    if (!wrapper)
        return NULL;

    wrapper->ssl = SSL_new(ctx);
    wrapper->ctx = ctx;
    wrapper->fd = fd;

    SSL_set_fd(wrapper->ssl, fd);
    if (SSL_accept(wrapper->ssl) <= 0)
    {
        ERR_print_errors_fp(stderr);
        free(wrapper);
        return NULL;
    }

    return wrapper;
}
SSLWrapper *ssl_wrapper_create_client(int fd, SSL_CTX *ctx)
{
    SSLWrapper *wrapper = malloc(sizeof(SSLWrapper));
    if (!wrapper)
        return NULL;

    wrapper->ssl = SSL_new(ctx);
    wrapper->ctx = ctx;
    wrapper->fd = fd;

    SSL_set_fd(wrapper->ssl, fd);
    if (SSL_connect(wrapper->ssl) <= 0) // <-- Client side
    {
        ERR_print_errors_fp(stderr);
        free(wrapper);
        return NULL;
    }

    return wrapper;
}
int ssl_wrapper_send(SSLWrapper *wrapper, const char *data, size_t len)
{
    return SSL_write(wrapper->ssl, data, len);
}

int ssl_wrapper_recv(SSLWrapper *wrapper, char *buffer, size_t len)
{
    return SSL_read(wrapper->ssl, buffer, len);
}

void ssl_wrapper_destroy(SSLWrapper *wrapper)
{
    if (wrapper)
    {
        SSL_shutdown(wrapper->ssl);
        SSL_free(wrapper->ssl);
        close(wrapper->fd);
        free(wrapper);
    }
}
/*------------------------------------------------------------------------ */

/*--------------------plain TCP (No security)------------------------------- */
// Plain TCP implementation (không bảo mật)
typedef struct
{
    int fd;
} PlainConnection;

static int plain_send(void *self, const char *data, size_t len)
{
    PlainConnection *conn = (PlainConnection *)self;
    return write(conn->fd, data, len);
}

static int plain_recv(void *self, char *buffer, size_t len)
{
    PlainConnection *conn = (PlainConnection *)self;
    return read(conn->fd, buffer, len);
}

static void plain_close(void *self)
{
    PlainConnection *conn = (PlainConnection *)self;
    close(conn->fd);
    free(conn);
}

SecureCommunication *create_secure_connection(int fd, SecureMode mode)
{
    SecureCommunication *comm = malloc(sizeof(SecureCommunication));
    if (!comm)
        return NULL;

    switch (mode)
    {
    case SECURE_SSL_SERVER:
    {
        SSLWrapper *wrapper = ssl_wrapper_create_server(fd, system_manager.ssl_context);
        if (!wrapper)
        {
            free(comm);
            return NULL;
        }
        comm->impl = wrapper;
        comm->interface.send = (int (*)(void *, const char *, size_t))ssl_wrapper_send;
        comm->interface.recv = (int (*)(void *, char *, size_t))ssl_wrapper_recv;
        comm->interface.close = (void (*)(void *))ssl_wrapper_destroy;
        break;
    }
    case SECURE_SSL_CLIENT:
    {
        SSLWrapper *wrapper = ssl_wrapper_create_client(fd, system_manager.ssl_context);
        if (!wrapper)
        {
            free(comm);
            return NULL;
        }
        comm->impl = wrapper;
        comm->interface.send = (int (*)(void *, const char *, size_t))ssl_wrapper_send;
        comm->interface.recv = (int (*)(void *, char *, size_t))ssl_wrapper_recv;
        comm->interface.close = (void (*)(void *))ssl_wrapper_destroy;
        break;
    }
    case SECURE_PLAIN:
    {
        PlainConnection *plain = malloc(sizeof(PlainConnection));
        if (!plain)
        {
            free(comm);
            return NULL;
        }
        plain->fd = fd;
        comm->impl = plain;
        comm->interface.send = plain_send;
        comm->interface.recv = plain_recv;
        comm->interface.close = plain_close;
        break;
    }
    }

    return comm;
}

void destroy_secure_connection(SecureCommunication *comm)
{
    if (comm)
    {
        comm->interface.close(comm->impl);
        free(comm);
    }
}
/*--------------------------------------------------------------------*/

/*--------------------SSL context-------------------------------------*/
void init_ssl_context()
{
    SSL_load_error_strings();
    SSL_library_init(); // if use OpenSSL 1.0.x
    OpenSSL_add_all_algorithms();

    const SSL_METHOD *method = SSLv23_server_method(); // fallback for old OpenSSL

    system_manager.ssl_context = SSL_CTX_new(method);
    if (!system_manager.ssl_context)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_certificate_file(system_manager.ssl_context, "cert.pem", SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(system_manager.ssl_context, "key.pem", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}
// void cleanup_ssl_context();
/*------------------------------------------------------------------------*/

/*-----------Protect DoS attack---------------------------------------------*/
static IpLimiterManager *get_ip_limiter_manager()
{
    return &system_manager.ip_limiter_manager;
}

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
/*-------------------------------------------------------------------------*/
