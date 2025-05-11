/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "utils.h"

/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
/**
 * \brief Retrieves local IP address
 *
 * Scans network interfaces and returns the first IP address in the 192.168.x.x range.
 * Used to determine local IP for peer-to-peer communication.
 *
 * \return Pointer to statically allocated string containing IP address, or NULL if not found.
 *
 * \note Only returns private IPs (192.168.x.x). Uses static buffer (not thread-safe).
 */
char *get_ip()
{
    struct ifaddrs *ifaddr, *ifa;
    static char ip_str[INET_ADDRSTRLEN];

    if (getifaddrs(&ifaddr) == -1)
    {
        log_msg(LOG_ERR, "getifaddrs");
        return NULL;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, ip_str, sizeof(ip_str));
            if (strncmp(ip_str, "192.168", 7) == 0)
            {
                freeifaddrs(ifaddr);
                return ip_str;
            }
        }
    }

    freeifaddrs(ifaddr);
    return NULL;
}
/**
 * \brief Thread-safe console printing
 *
 * Prints formatted output to console using a mutex to avoid race conditions when
 * multiple threads write concurrently.
 *
 * \param ctx Application context containing console mutex
 * \param format printf-style format string
 * \param ... Additional arguments for format string
 */
void print_sync(AppContext *ctx, const char *format, ...)
{
    pthread_mutex_lock(&ctx->console_mutex);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    pthread_mutex_unlock(&ctx->console_mutex);
}
/**
 * \brief Checks if a peer connection already exists
 *
 * Scans current peer list to determine if a connection with given IP and port is active.
 *
 * \param manager PeerConnectionManager containing current peer connections
 * \param ip IP address to check
 * \param port Port number to check
 * \return true if the peer connection exists, false otherwise
 *
 * \note Thread-safe: uses connection_mutex internally
 */
bool is_peer_exists(PeerConnectionManager *manager, const char *ip, uint16_t port)
{
    if (!manager || !ip)
        return false;

    pthread_mutex_lock(&manager->connection_mutex);
    for (int i = 0; i < MAX_PEERS; ++i)
    {
        PeerConnection *p = &manager->peers[i];
        if (p->active && strcmp(p->ip_address, ip) == 0 && p->port == port)
        {
            pthread_mutex_unlock(&manager->connection_mutex);
            return true;
        }
    }
    pthread_mutex_unlock(&manager->connection_mutex);
    return false;
}
/**
 * \brief Validates IP address and port
 *
 * Ensures IP is non-empty, well-formed, and port is in valid range (1024–65535).
 * Prevents connecting to self or duplicate peers.
 *
 * \param ctx Application context, used for self-check and logging
 * \param ip IP address to validate
 * \param port Port number to validate
 * \return true if IP and port are valid and not already connected, false otherwise
 *
 * \note Logs errors for invalid input. Rejects connection to self or already connected peers.
 */
bool is_valid_ip_and_port(AppContext *ctx, const char *ip, uint16_t port)
{
    if (!ip || strlen(ip) == 0)
    {
        log_msg(LOG_ERR, "IP address is empty.");
        return false;
    }

    struct sockaddr_in sa;
    if (inet_pton(AF_INET, ip, &(sa.sin_addr)) != 1)
    {
        log_msg(LOG_ERR, "Invalid IP address format: %s", ip);
        return false;
    }

    if (port < 1024 || port > 65535)
    {
        log_msg(LOG_ERR, "Port must be in range 1024 - 65535. Got: %d", port);
        return false;
    }

    if (strcmp(ip, ctx->local_info.ip_address) == 0 && port == ctx->local_info.port)
    {
        log_msg(LOG_ERR, "Cannot connect to self (same IP and port).");
        return false;
    }

    // check if connected with ip and port
    if (is_peer_exists(ctx->peer_manager, ip, port))
    {
        log_msg(LOG_WARN, "Already connected to %s:%d", ip, port);
        return false;
    }

    return true;
}
