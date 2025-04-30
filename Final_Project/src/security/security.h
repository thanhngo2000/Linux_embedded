#ifndef SECURITY_H
#define SECURITY_H
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "../../include/shared_data.h"
/******************************************************************************/
/*                            FUNCTIONS PROTOTYPES                             */
/******************************************************************************/
SecureCommunication *create_secure_connection(int fd, SecureMode mode);
void destroy_secure_connection(SecureCommunication *comm);
void init_ssl_context();
void cleanup_ssl_context();
// SSL_CTX *init_ssl_context()

void init_ip_limiter_manager();
bool ip_limiter_allow_connection(const char *ip);
void ip_limiter_remove_connection(const char *ip);

#endif