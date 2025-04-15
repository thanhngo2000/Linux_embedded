#ifndef SECURITY_H
#define SECURITY_H
#include "../../include/shared_data.h"

typedef struct
{
    SSL *ssl;
    SSL_CTX *ctx;
    int fd;
} SSLWrapper;

SSLWrapper *ssl_wrapper_create(int fd, SSL_CTX *ctx);
void ssl_wrapper_destroy(SSLWrapper *wrapper);
int ssl_wrapper_send(SSLWrapper *wrapper, const char *data, size_t len);
int ssl_wrapper_recv(SSLWrapper *wrapper, char *buffer, size_t len);

SecureCommunication *create_secure_connection(int fd, SecureMode mode);
void destroy_secure_connection(SecureCommunication *comm);

void init_ssl_context();

void init_ip_limiter_manager();
bool ip_limiter_allow_connection(const char *ip);
void ip_limiter_remove_connection(const char *ip);

#endif