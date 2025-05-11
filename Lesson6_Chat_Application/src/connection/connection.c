/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "connection.h"
/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
/**
 * \brief Creates a new PeerConnectionManager instance
 *
 * Allocates and initializes a new PeerConnectionManager with zeroed memory
 * and initializes the connection mutex.
 *
 * \return Pointer to new manager or NULL if allocation failed
 */
PeerConnectionManager *create_peer_manager()
{
    PeerConnectionManager *manager = malloc(sizeof(PeerConnectionManager));
    if (!manager)
        return NULL;

    memset(manager, 0, sizeof(PeerConnectionManager));
    pthread_mutex_init(&manager->connection_mutex, NULL);
    return manager;
}
/**
 * \brief Initializes a PeerConnectionManager instance
 *
 * Resets all peer slots to inactive state and initializes connection mutex.
 * Logs error if manager is NULL or mutex initialization fails.
 *
 * \param manager Pointer to manager to initialize
 */
void init_peer_manager(PeerConnectionManager *manager)
{
    if (manager == NULL)
    {
        log_msg(LOG_ERR, "init_peer_manager: NULL manager provided.");
        return;
    }

    manager->connection_count = 0;

    for (int i = 0; i < MAX_PEERS; ++i)
    {
        manager->peers[i].id = 0;
        manager->peers[i].socket_fd = -1;
        manager->peers[i].ip_address[0] = '\0';
        manager->peers[i].port = 0;
        manager->peers[i].active = false;
    }

    if (pthread_mutex_init(&manager->connection_mutex, NULL) != 0)
    {
        log_msg(LOG_ERR, "init_peer_manager: Failed to initialize mutex.");
    }
}
/**
 * \brief Destroys a PeerConnectionManager instance
 *
 * Closes all active connections, destroys mutex, and frees manager memory.
 * Does nothing if manager is NULL.
 *
 * \param manager Pointer to manager to destroy
 */
void destroy_peer_manager(PeerConnectionManager *manager)
{
    if (!manager)
        return;

    close_all_peer_manager(manager);
    pthread_mutex_destroy(&manager->connection_mutex);
    free(manager);
}
/**
 * \brief Adds a new peer connection to the manager
 *
 * Adds peer with given IP, port and socket fd to first available slot.
 * Thread-safe operation protected by mutex.
 *
 * \param manager Pointer to manager
 * \param ip Peer IP address
 * \param port Peer port number
 * \param sock_fd Socket file descriptor
 * \return true if added successfully, false if full or invalid params
 */
bool add_peer_manager(PeerConnectionManager *manager, const char *ip, uint16_t port, int sock_fd)
{
    if (!manager || !ip)
        return false;

    pthread_mutex_lock(&manager->connection_mutex);
    if (manager->connection_count >= MAX_PEERS)
    {
        pthread_mutex_unlock(&manager->connection_mutex);
        return false;
    }

    for (int i = 0; i < MAX_PEERS; ++i)
    {
        if (!manager->peers[i].active)
        {

            manager->peers[i].id = manager->connection_count;
            manager->peers[i].socket_fd = sock_fd;
            strncpy(manager->peers[i].ip_address, ip, MAX_IP_ADDRESS_LENGTH);
            manager->peers[i].port = port;
            manager->peers[i].active = true;
            manager->connection_count++;
            break;
        }
    }

    pthread_mutex_unlock(&manager->connection_mutex);
    return true;
}
/**
 * \brief Removes peer by connection ID
 *
 * Closes connection and removes peer from manager by ID.
 * Compacts the peer list after removal.
 *
 * \param manager Pointer to manager
 * \param id Connection ID to remove
 * \return true if found and removed, false otherwise
 */
bool remove_peer_by_id(PeerConnectionManager *manager, int id)
{
    if (!manager)
        return false;

    pthread_mutex_lock(&manager->connection_mutex);
    for (int i = 0; i < MAX_PEERS; ++i)
    {
        if (manager->peers[i].active && manager->peers[i].id == id)
        {
            close(manager->peers[i].socket_fd);

            // move data to up
            for (int j = i; j < MAX_PEERS - 1; ++j)
            {
                manager->peers[j] = manager->peers[j + 1];
                manager->peers[j].id = j;
            }

            // delete final param
            memset(&manager->peers[MAX_PEERS - 1], 0, sizeof(PeerConnection));
            manager->peers[MAX_PEERS - 1].socket_fd = -1;

            manager->connection_count--;
            pthread_mutex_unlock(&manager->connection_mutex);
            return true;
        }
    }
    pthread_mutex_unlock(&manager->connection_mutex);
    return false;
}
/**
 * \brief Removes peer by socket file descriptor
 *
 * Closes connection and marks peer as inactive by socket fd.
 *
 * \param manager Pointer to manager
 * \param socket_fd Socket fd to remove
 * \return true if found and removed, false otherwise
 */
bool remove_peer_by_socket(PeerConnectionManager *manager, int socket_fd)
{
    pthread_mutex_lock(&manager->connection_mutex);
    for (int i = 0; i < MAX_PEERS; i++)
    {
        if (manager->peers[i].active && manager->peers[i].socket_fd == socket_fd)
        {
            close(manager->peers[i].socket_fd);
            manager->peers[i].active = false;
            manager->connection_count--;
            pthread_mutex_unlock(&manager->connection_mutex);
            return true;
        }
    }
    pthread_mutex_unlock(&manager->connection_mutex);
    return false;
}
/**
 * \brief Closes all active peer connections
 *
 * Closes all sockets and marks all peers as inactive.
 * Resets connection count to 0.
 *
 * \param manager Pointer to manager
 */
void close_all_peer_manager(PeerConnectionManager *manager)
{
    if (!manager)
        return;

    pthread_mutex_lock(&manager->connection_mutex);
    for (int i = 0; i < MAX_PEERS; ++i)
    {
        if (manager->peers[i].active)
        {
            close(manager->peers[i].socket_fd);
            manager->peers[i].active = false;
        }
    }
    manager->connection_count = 0;
    pthread_mutex_unlock(&manager->connection_mutex);
}
/**
 * \brief Gets current active peer count
 *
 * Thread-safe operation protected by mutex.
 *
 * \param manager Pointer to manager
 * \return Number of active connections (0 if manager is NULL)
 */
int count_peer_manager(PeerConnectionManager *manager)
{
    if (!manager)
        return 0;

    pthread_mutex_lock(&manager->connection_mutex);
    int count = manager->connection_count;
    pthread_mutex_unlock(&manager->connection_mutex);
    return count;
}
/**
 * \brief Gets peer connection by array index
 *
 * Returns pointer to peer at specified index if active.
 * Thread-safe operation protected by mutex.
 *
 * \param manager Pointer to manager
 * \param index Array index (0-MAX_PEERS)
 * \return Pointer to PeerConnection or NULL if invalid
 */
PeerConnection *get_peer_by_index(PeerConnectionManager *manager, const int index)
{
    if (!manager || index < 0 || index >= MAX_PEERS)
        return NULL;

    pthread_mutex_lock(&manager->connection_mutex);
    PeerConnection *peer = NULL;
    if (manager->peers[index].active)
    {
        peer = &manager->peers[index];
    }
    pthread_mutex_unlock(&manager->connection_mutex);

    return peer;
}