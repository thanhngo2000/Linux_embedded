/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "connection.h"
/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
/**
 * \brief Creates a new node for the connection list.
 *
 * Allocates memory for a new ConnectionNode, initializes its fields with the provided
 * sensor connection and sensor data, and sets the next pointer to NULL.
 * If memory allocation fails, returns NULL.
 *
 * \param connection The SensorConnection structure containing the details of the sensor connection.
 * \param data The SensorData structure containing the latest data for the sensor connection.
 *
 * \return ConnectionNode* Pointer to the newly created node, or NULL if memory allocation fails.
 */
static ConnectionNode *create_node(SensorConnection connection, SensorData data)
{
    ConnectionNode *node = (ConnectionNode *)malloc(sizeof(ConnectionNode));
    if (!node)
        return NULL;
    node->connection = connection;
    node->latest_data = data;
    node->next = NULL;
    return node;
}

/**
 * \brief Adds a new sensor connection to the linked list of active connections.
 *
 * Verifies IP rate limits before adding. Updates the global connection manager and ensures thread safety.
 *
 * \param head Double pointer to the head of the connection list.
 * \param connection The sensor connection to add.
 * \param data The initial sensor data.
 *
 * \return void
 */
static void add_connection(ConnectionNode **head, SensorConnection connection, SensorData data)
{
    // check IP limiter before connection
    if (!ip_limiter_allow_connection(connection.ip_address))
    {
        printf("🚫 connection from IP %s rejeccted (over limit).\n", connection.ip_address);
        return;
    }

    // add new node connection
    ConnectionNode *new_node = create_node(connection, data);
    if (!new_node)
    {
        handle_error("Failed add new connection");
        return;
    }

    pthread_mutex_lock(&system_manager.connection_manager.mutex);

    new_node->next = *head;
    *head = new_node;
    system_manager.connection_manager.active_count++;

    pthread_mutex_unlock(&system_manager.connection_manager.mutex);
}
/**
 * \brief Removes a sensor connection from the list by sensor ID.
 *
 * Releases associated resources including socket and memory,
 * and updates the active connection count.
 *
 * \param head Double pointer to the head of the connection list.
 * \param sensor_id The ID of the sensor to remove.
 *
 * \return void
 */
static void remove_connection(ConnectionNode **head, int sensor_id)
{
    pthread_mutex_lock(&system_manager.connection_manager.mutex);

    ConnectionNode *current = *head, *prev = NULL;
    while (current)
    {
        if (current->connection.sensor_id == sensor_id)
        {
            // remove ip from security
            ip_limiter_remove_connection(current->connection.ip_address);

            if (prev)
                prev->next = current->next;
            else
                *head = current->next;

            // close socket
            if (current->connection.socket_fd >= 0)
            {
                if (current->connection.secure_comm)
                {
                    destroy_secure_connection(current->connection.secure_comm);
                    current->connection.secure_comm = NULL;
                }
                close(current->connection.socket_fd); // user add
            }

            free(current);
            system_manager.connection_manager.active_count--;
            break;
        }
        prev = current;
        current = current->next;
    }

    pthread_mutex_unlock(&system_manager.connection_manager.mutex);
}

/**
 * \brief Finds a connection node in the list by sensor ID.
 *
 * \param head Pointer to the head of the connection list.
 * \param sensor_id The ID of the sensor to search for.
 *
 * \return ConnectionNode* Pointer to the found node, or NULL if not found.
 */
static ConnectionNode *find_connection(ConnectionNode *head, int sensor_id)
{
    // pthread_mutex_lock(&system_manager.connection_manager.mutex);
    while (head)
    {
        if (head->connection.sensor_id == sensor_id)
        {
            // pthread_mutex_unlock(&system_manager.connection_manager.mutex);
            return head;
        }

        head = head->next;
    }
    // pthread_mutex_unlock(&system_manager.connection_manager.mutex);
    return NULL;
}
/**
 * \brief Updates the latest sensor data and timestamp for a specific sensor connection.
 *
 * \param head Pointer to the head of the connection list.
 * \param sensor_id The ID of the sensor whose data is being updated.
 * \param data The new sensor data to assign.
 *
 * \return void
 */
static void update_data_connection(ConnectionNode *head, int sensor_id, SensorData data)
{
    pthread_mutex_lock(&system_manager.connection_manager.mutex);

    ConnectionNode *node = find_connection(head, sensor_id);
    if (node)
    {
        node->latest_data = data;
        node->connection.last_active_time = time(NULL);
    }

    pthread_mutex_unlock(&system_manager.connection_manager.mutex);
}

/*---------------Function print data connection-----------------------------------------*/
/**
 * \brief Converts a connection status enum value to a human-readable string.
 *
 * Maps the connection status code to its corresponding string representation,
 * such as "CONNECTED", "DISCONNECTED", "TIMEOUT", or "UNKNOWN" for invalid values.
 *
 * \param status ConnectionStatus enum value representing the connection's status.
 *
 * \return const char* Pointer to the string representing the connection status.
 */
static const char *connection_status_to_string(ConnectionStatus status)
{
    switch (status)
    {
    case CONN_STATUS_CONNECTED:
        return "CONNECTED";
    case CONN_STATUS_DISCONNECTED:
        return "DISCONNECTED";
    case CONN_STATUS_TIMEOUT:
        return "TIMEOUT";
    default:
        return "UNKNOWN";
    }
}
/**
 * \brief Formats a raw time value into a human-readable timestamp string.
 *
 * Converts the given time_t value to a string in the "YYYY-MM-DD HH:MM:SS" format.
 * If the conversion fails, outputs "N/A" instead.
 *
 * \param raw_time The raw time value to be formatted.
 * \param buffer Pointer to the character buffer where the formatted string will be stored.
 * \param buffer_size Size of the buffer to prevent overflow.
 *
 * \return void
 */
static void format_time(time_t raw_time, char *buffer, size_t buffer_size)
{
    struct tm *time_info = localtime(&raw_time);
    if (time_info != NULL)
    {
        strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", time_info);
    }
    else
    {
        snprintf(buffer, buffer_size, "N/A");
    }
}
/**
 * \brief Prints the information of a single sensor connection in a formatted table row.
 *
 * Formats the connection status and timestamps before displaying the sensor ID, IP address,
 * port, status, connected time, and last active time.
 *
 * \param conn Pointer to the SensorConnection structure containing connection details.
 *
 * \return void
 */
static void print_connection_info(const SensorConnection *conn)
{
    char connected_time_str[20];
    char last_active_time_str[20];

    format_time(conn->connected_time, connected_time_str, sizeof(connected_time_str));
    format_time(conn->last_active_time, last_active_time_str, sizeof(last_active_time_str));

    const char *status_str = connection_status_to_string(conn->status);

    printf("| %4d | %17s | %5d | %10s | %19s | %19s |\n",
           conn->sensor_id,
           conn->ip_address,
           conn->port,
           status_str,
           connected_time_str,
           last_active_time_str);
}

/**
 * \brief Displays all currently active sensor connections in a formatted table.
 *
 * Includes connection ID, IP address, port, status, connected time, and last active time.
 * Ensures thread-safe access to the connection list.
 *
 * \param head Pointer to the head of the connection list.
 *
 * \return void
 */
void display_active_connections(ConnectionNode *head)
{
    pthread_mutex_lock(&system_manager.connection_manager.mutex);

    printf("\n=== ACTIVE CONNECTIONS (%d) ===\n", system_manager.connection_manager.active_count);
    printf("+------+-------------------+-------+------------+---------------------+---------------------+\n");
    printf("|  ID  |      IP Address   | Port  |   Status   |    Connected Time   |   Last Active Time  |\n");
    printf("+------+-------------------+-------+------------+---------------------+---------------------+\n");

    ConnectionNode *current = head;
    while (current != NULL)
    {
        print_connection_info(&current->connection);
        current = current->next;
    }

    printf("+------+-------------------+-------+------------+---------------------+---------------------+\n");
    pthread_mutex_unlock(&system_manager.connection_manager.mutex);
}
/*-----------------------------------------------------------------------------------------*/
/**
 * \brief Displays basic resource usage statistics such as RAM and CPU cores.
 *
 * Uses the sysinfo system call to retrieve and display memory and CPU usage.
 *
 * \return void
 */
static void display_resource_usage()
{
    struct sysinfo info;
    if (sysinfo(&info) == 0)
    {
        unsigned long total_ram_mb = info.totalram / 1024 / 1024;
        unsigned long used_ram_mb = (info.totalram - info.freeram) / 1024 / 1024;

        printf(" RAM: %lu MB used / %lu MB total\n", used_ram_mb, total_ram_mb);

        long cpu_cores = sysconf(_SC_NPROCESSORS_ONLN);
        printf("CPU cores: %ld\n", cpu_cores);
    }
    else
    {
        perror("sysinfo");
    }
}
/**
 * \brief Displays overall system status including active connections and message statistics.
 *
 * Gathers and prints information from connection and storage managers, including RAM/CPU usage.
 *
 * \return void
 */
void display_system_status()
{
    int active_connections = 0;
    int total_messages_conn = 0;
    int total_messages_db = 0;

    // num sensor
    pthread_mutex_lock(&system_manager.connection_manager.mutex);
    active_connections = system_manager.connection_manager.active_count;
    pthread_mutex_unlock(&system_manager.connection_manager.mutex);

    // num message receive
    pthread_mutex_lock(&system_manager.storage_manager.mutex);
    total_messages_db = system_manager.storage_manager.total_messages_received;
    pthread_mutex_unlock(&system_manager.storage_manager.mutex);

    printf("\n[System Status]\n");
    printf("Active connections       : %d\n", active_connections);
    printf(" Total messages received : %d (live buffer: %d)\n", total_messages_db, total_messages_conn);
    display_resource_usage();
}
/**

\brief Initializes the connection manager by setting initial values for connection list,
active count, running port, and mutex.

This function initializes the connection manager with default values,
including setting the head of the connection list to NULL,

active connection count to 0, and running port to 0. Additionally,
it initializes a mutex for thread safety and binds the connection

management methods (add, remove, find, update, display) to the respective functions.

\return void */
void init_connection_manager()
{
    system_manager.connection_manager.head = NULL;
    system_manager.connection_manager.active_count = 0;
    system_manager.connection_manager.running_port = 0;
    pthread_mutex_init(&system_manager.connection_manager.mutex, NULL);

    // Bind methods (OOP-style)
    system_manager.connection_manager.add = add_connection;
    system_manager.connection_manager.remove = remove_connection;
    system_manager.connection_manager.find = find_connection;
    system_manager.connection_manager.update = update_data_connection;
    system_manager.connection_manager.display = display_active_connections;
}
/**

\brief Cleans up the resources used by the connection manager.

This function releases the memory allocated for each connection node in the connection list and resets the connection manager's

internal states (head, active count, running port). The mutex is unlocked and destroyed to free associated resources.

\return void */
void cleanup_connection_manager()
{
    pthread_mutex_lock(&system_manager.connection_manager.mutex);

    ConnectionNode *current = system_manager.connection_manager.head;
    while (current != NULL)
    {
        ConnectionNode *tmp = current;
        current = current->next;

        // release secure_comm
        if (tmp->connection.secure_comm != NULL)
        {
            // destroy
            destroy_secure_connection(tmp->connection.secure_comm);
            tmp->connection.secure_comm = NULL;
        }

        // close socket
        if (tmp->connection.socket_fd >= 0)
        {
            close(tmp->connection.socket_fd);
        }

        free(tmp);
    }

    system_manager.connection_manager.head = NULL;
    system_manager.connection_manager.active_count = 0;
    system_manager.connection_manager.running_port = 0;

    pthread_mutex_unlock(&system_manager.connection_manager.mutex);
    pthread_mutex_destroy(&system_manager.connection_manager.mutex);
}
/*------------------------handle new connection-----------------*/
/**

\brief Creates a new sensor connection from the client information and assigns a sensor ID.

\param packet Pointer to the ClientInfoPacket with client info.

\param sensor_id The unique sensor ID for the connection.

\return A new SensorConnection initialized with provided data. */
// static SensorConnection create_sensor_connection(ClientInfoPacket *packet, int sensor_id)
static SensorConnection create_sensor_connection(ClientInfoPacket *packet, int sensor_id, SecureCommunication *secure_comm)
{
    SensorConnection conn = {
        .socket_fd = packet->sock_fd,
        .port = packet->port,
        .sensor_id = sensor_id,
        .is_active = true,
        .status = CONN_STATUS_CONNECTED,
        .connected_time = time(NULL),
        .last_active_time = time(NULL),
        .secure_comm = secure_comm,
    };
    strncpy(conn.ip_address, packet->ip_address, INET_ADDRSTRLEN);
    return conn;
}
/**

\brief Creates initial sensor data with default values.

\param sensor_id The unique sensor ID for the data.

\return A SensorData struct with default values. */

static SensorData create_initial_sensor_data(int sensor_id)
{
    return (SensorData){
        .sensor_id = sensor_id,
        .timestamp = time(NULL),
        .temperature = 0.0f,
        .is_valid = false};
}

/**

\brief Updates the sensor temperature and stores the new data.

\param sensor_id The sensor ID to update.

\param temperature The new temperature value.

\return void */
static void update_sensor_temperature(int sensor_id, float temperature)
{
    time_t current_time = time(NULL);

    SensorData new_data = {
        .timestamp = current_time,
        .sensor_id = sensor_id,
        .temperature = temperature,
        .is_valid = true};

    // update to connection manager
    ConnectionNode *node = system_manager.connection_manager.find(
        system_manager.connection_manager.head, sensor_id);
    if (node)
    {
        node->latest_data = new_data;
        node->connection.last_active_time = current_time;
    }

    // store data
    storage_add_data(new_data);
}
/**
 * \brief Receives all data from a secure communication channel.
 *
 * This function repeatedly calls the `recv` method of the communication interface
 * until the requested number of bytes are received or an error occurs.
 *
 * \param comm The secure communication interface.
 * \param buffer The buffer to store the received data.
 * \param size The number of bytes to receive.
 *
 * \return 0 on success, -1 if an error occurs or the connection is closed.
 */
static int recv_all(SecureCommunication *comm, void *buffer, size_t size)
{
    size_t total_received = 0;
    while (total_received < size)
    {
        int bytes_received = comm->interface.recv(comm->impl, (char *)buffer + total_received, size - total_received);
        if (bytes_received <= 0)
        {
            return -1; // Error or connection closed
        }
        total_received += bytes_received;
    }
    return 0; // OK
}

/**

\brief Handles a new client connection, validates and adds it to the connection manager.

\param epoll_fd The epoll file descriptor.

\param server_fd The server socket file descriptor.

\return void */
void handle_new_connection(int epoll_fd, int server_fd)
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char client_ip[INET_ADDRSTRLEN];

    int client_fd = accept_client(server_fd, &client_addr, &client_len);
    if (client_fd < 0)
    {
        handle_error("Client fd accept fail");
        return;
    }

    SecureCommunication *comm = create_secure_connection(client_fd, SECURE_SSL_SERVER);
    if (!comm)
    {
        handle_error("Fail accept client with security");
        close(client_fd);
        return;
    }

    ClientInfoPacket packet;
    if (recv_all(comm, &packet, sizeof(packet)) < 0)
    {
        handle_error("Failed to read client info securely");
        destroy_secure_connection(comm);
        close(client_fd);
        return;
    }

    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    warn_if_ip_mismatch(client_ip, packet.ip_address);

    if (is_port_already_connected(packet.port))
    {
        handle_error("Port already connected");
        destroy_secure_connection(comm);
        close(client_fd);
        return;
    }

    int sensor_id = system_manager.connection_manager.active_count;
    SensorConnection conn = create_sensor_connection(&packet, sensor_id, comm);
    SensorData init_data = create_initial_sensor_data(sensor_id);

    system_manager.connection_manager.add(&system_manager.connection_manager.head, conn, init_data);

    // Log new connection
    char msg[256];
    snprintf(msg, sizeof(msg), "A sensor node with %d has opened a new connection", sensor_id);
    system_manager.log_manager.log(&system_manager.log_manager, LOG_INFO, "Connection", msg);

    if (!add_client_fd_to_epoll(epoll_fd, client_fd))
    {
        system_manager.connection_manager.remove(&system_manager.connection_manager.head, sensor_id);
        snprintf(msg, sizeof(msg), "A sensor node with %d has closed the connection", sensor_id);
        system_manager.log_manager.log(&system_manager.log_manager, LOG_INFO, "Connection", msg);
        destroy_secure_connection(comm);
        close(client_fd);
        return;
    }

    printf("New connection ip:%s port: %d (ID: %d)\n", packet.ip_address, packet.port, sensor_id);
}

/**

\brief Updates the sensor data for all active connections every 3 seconds.

\details This function iterates through all active sensor connections, generates a random temperature value for each, and updates the sensor data.

\param arg Pointer to any argument (not used in this case).

\return NULL */
void *update_sensor_data_thread(void *arg)
{
    while (1)
    {

        ConnectionNode *current = system_manager.connection_manager.head;
        while (current != NULL)
        {
            if (current->connection.is_active)
            {
                // create random temperature (0-50°C)
                float new_temp = (float)(rand() % 50);
                update_sensor_temperature(current->connection.sensor_id, new_temp);
            }
            current = current->next;
        }

        sleep(3); // update after 3 seconds
    }
    return NULL;
}

/**

\brief Checks if a sensor connection has timed out.

\details This function compares the current time with the last active time of the sensor connection.
If the difference exceeds the timeout threshold, it returns true.

\param conn Pointer to the SensorConnection to check.

\param now The current time to compare against.

\return true if the connection has timed out, false otherwise. */
static bool is_connection_timed_out(SensorConnection *conn, time_t now)
{
    return conn->is_active && difftime(now, conn->last_active_time) > CONNECTION_TIMEOUT_SECONDS;
}
/**

\brief Cleans up inactive sensor connections that have timed out.

\details This function iterates through the list of connections and removes those that have timed out based on the CONNECTION_TIMEOUT_SECONDS threshold.

The corresponding socket is closed, and resources are freed.

\return void */
static void check_and_cleanup_inactive_connections()
{
    time_t now = time(NULL);
    ConnectionManager *manager = &system_manager.connection_manager;

    pthread_mutex_lock(&manager->mutex);

    ConnectionNode *prev = NULL;
    ConnectionNode *current = manager->head;

    while (current)
    {
        ConnectionNode *next = current->next;

        if (is_connection_timed_out(&current->connection, now))
        {
            // terminate connection
            int sensor_id = current->connection.sensor_id;
            printf("[TIMEOUT] Sensor ID %d disconnected due to inactivity.\n", sensor_id);
            close(current->connection.socket_fd);
            ip_limiter_remove_connection(current->connection.ip_address);

            if (prev)
                prev->next = next;
            else
                manager->head = next;

            free(current);
            manager->active_count--;

            char msg[256];
            snprintf(msg, sizeof(msg), "A sensor node with %d has closed the connection", sensor_id);
            system_manager.log_manager.log(&system_manager.log_manager, LOG_INFO, "Connection", msg);
        }
        else
        {
            prev = current;
        }

        current = next;
    }

    pthread_mutex_unlock(&manager->mutex);
}
/**

\brief Periodically checks and cleans up inactive connections every 5 seconds.

\details This function runs in a separate thread and calls check_and_cleanup_inactive_connections to handle the removal of timed-out connections every 5 seconds.

\param arg Pointer to any argument (not used in this case).

\return NULL */
void *check_timeout_connection_thread(void *arg)
{
    while (1)
    {
        check_and_cleanup_inactive_connections();
        sleep(5); // check again after 5 seconds
    }
    return NULL;
}
/**
 * \brief Sets the currently running port in the connection manager.
 *
 * This function updates the `running_port` value in a thread-safe manner
 * using a mutex lock.
 *
 * \param port The port number to set as running.
 *
 * \return void
 */
static void set_running_port(int port)
{
    pthread_mutex_lock(&system_manager.connection_manager.mutex);
    system_manager.connection_manager.running_port = port;
    pthread_mutex_unlock(&system_manager.connection_manager.mutex);
}
/**
 * \brief Finds a connection node by its socket file descriptor.
 *
 * This function traverses the linked list of connections in a thread-safe
 * way to locate the connection associated with the given file descriptor.
 *
 * \param client_fd The socket file descriptor to search for.
 *
 * \return A pointer to the found ConnectionNode, or NULL if not found.
 */
static ConnectionNode *find_connection_by_fd(int client_fd)
{
    pthread_mutex_lock(&system_manager.connection_manager.mutex);
    ConnectionNode *curr = system_manager.connection_manager.head;
    while (curr && curr->connection.socket_fd != client_fd)
        curr = curr->next;
    pthread_mutex_unlock(&system_manager.connection_manager.mutex);
    return curr;
}
/**
 * \brief Finds a connection node by its socket file descriptor.
 *
 * This function traverses the linked list of connections in a thread-safe
 * way to locate the connection associated with the given file descriptor.
 *
 * \param client_fd The socket file descriptor to search for.
 *
 * \return A pointer to the found ConnectionNode, or NULL if not found.
 */
static void handle_existing_connection(int epoll_fd, int client_fd)
{
    ConnectionNode *conn = find_connection_by_fd(client_fd);
    if (!conn)
        return;

    char buffer[1];
    int result = recv(client_fd, buffer, sizeof(buffer), MSG_PEEK);

    if (result == 0)
    {
        char msg[256];
        snprintf(msg, sizeof(msg), "Sensor %d disconnected", conn->connection.sensor_id);
        system_manager.log_manager.log(&system_manager.log_manager, LOG_INFO, "Connection", msg);

        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
        system_manager.connection_manager.remove(&system_manager.connection_manager.head, conn->connection.sensor_id);
    }
    else if (result < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
    {
        perror("recv");
        char msg[256];
        snprintf(msg, sizeof(msg), "Sensor %d error disconnect: %s", conn->connection.sensor_id, strerror(errno));
        system_manager.log_manager.log(&system_manager.log_manager, LOG_ERROR, "Connection", msg);

        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
        system_manager.connection_manager.remove(&system_manager.connection_manager.head, conn->connection.sensor_id);
    }
}
/**

\brief Manages the connection setup and handling for the server.

\details This function initializes the server socket, listens for incoming connections, and handles new connections using epoll. It also tracks the active port.

\param arg Pointer to the port number on which to bind the server.

\return NULL */
void *connection_manager(void *arg)
{
    int port = *(int *)arg;

    int server_fd = create_and_bind_socket(port);
    if (server_fd < 0)
        return NULL;

    set_running_port(port);

    if (listen(server_fd, 10) < 0)
    {
        perror("Error listen socket");
        exit(EXIT_FAILURE);
    }

    int epoll_fd = setup_epoll(server_fd);
    if (epoll_fd == -1)
    {
        close(server_fd);
        return NULL;
    }

    struct epoll_event events[10];
    while (!stop_requested)
    {
        int nfds = epoll_wait(epoll_fd, events, 10, -1);
        if (nfds == -1)
        {
            if (errno == EINTR)
                continue;
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nfds; ++i)
        {
            int fd = events[i].data.fd;
            if (fd == server_fd)
            {
                handle_new_connection(epoll_fd, server_fd);
            }
            else
            {
                handle_existing_connection(epoll_fd, fd);
            }
        }
    }

    close(epoll_fd);
    close(server_fd);
    return NULL;
}