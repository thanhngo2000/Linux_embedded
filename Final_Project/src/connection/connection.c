/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "connection.h"
/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
/**
 * \brief Creates a new connection node with the given sensor connection and data.
 *
 * \param connection The sensor connection information.
 * \param data The initial sensor data associated with the connection.
 *
 * \return ConnectionNode* Pointer to the newly allocated node, or NULL on failure.
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
    // Kiểm tra giới hạn IP trước khi thêm kết nối
    if (!ip_limiter_allow_connection(connection.ip_address))
    {
        printf("🚫 Kết nối từ IP %s bị từ chối (vượt quá giới hạn).\n", connection.ip_address);
        return;
    }

    ConnectionNode *new_node = create_node(connection, data);
    if (!new_node)
        return;

    pthread_mutex_lock(&system_manager.connection_manager.mutex);

    new_node->next = *head;
    *head = new_node;
    system_manager.connection_manager.active_count++;

    pthread_mutex_unlock(&system_manager.connection_manager.mutex);
}
/**
 * \brief Removes a sensor connection from the list by sensor ID.
 *
 * Releases associated resources including socket and memory, and updates the active connection count.
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
                close(current->connection.socket_fd); // user add
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
    pthread_mutex_lock(&system_manager.connection_manager.mutex);
    while (head)
    {
        if (head->connection.sensor_id == sensor_id)
        {
            pthread_mutex_unlock(&system_manager.connection_manager.mutex);
            return head;
        }

        head = head->next;
    }
    pthread_mutex_unlock(&system_manager.connection_manager.mutex);
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
static void display_active_connections(ConnectionNode *head)
{
    pthread_mutex_lock(&system_manager.connection_manager.mutex);

    printf("\n=== ACTIVE CONNECTIONS (%d) ===\n", system_manager.connection_manager.active_count);
    printf("+------+-------------------+-------+------------+---------------------+---------------------+\n");
    printf("|  ID  |      IP Address   | Port  |   Status   |    Connected Time   |   Last Active Time  |\n");
    printf("+------+-------------------+-------+------------+---------------------+---------------------+\n");

    // time_t now = time(NULL);
    ConnectionNode *current = head;

    while (current != NULL)
    {
        SensorConnection conn = current->connection;

        // change time
        char connected_time_str[20];
        char last_active_str[20];
        strftime(connected_time_str, sizeof(connected_time_str), "%Y-%m-%d %H:%M:%S", localtime(&conn.connected_time));
        strftime(last_active_str, sizeof(last_active_str), "%Y-%m-%d %H:%M:%S", localtime(&conn.last_active_time));

        // change status to string
        const char *status_str;
        switch (conn.status)
        {
        case CONN_STATUS_CONNECTED:
            status_str = "CONNECTED";
            break;
        case CONN_STATUS_DISCONNECTED:
            status_str = "DISCONNECTED";
            break;
        case CONN_STATUS_TIMEOUT:
            status_str = "TIMEOUT";
            break;
        default:
            status_str = "UNKNOWN";
            break;
        }

        // display connection info
        printf("| %4d | %17s | %5d | %10s | %19s | %19s |\n",
               conn.sensor_id,
               conn.ip_address,
               conn.port,
               status_str,
               connected_time_str,
               last_active_str);

        current = current->next;
    }

    printf("+------+-------------------+-------+------------+---------------------+---------------------+\n");
    pthread_mutex_unlock(&system_manager.connection_manager.mutex);
}

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

    // In ra
    printf("\n[System Status]\n");
    printf("Active connections       : %d\n", active_connections);
    printf(" Total messages received : %d (live buffer: %d)\n", total_messages_db, total_messages_conn);
    display_resource_usage();
}
/**

\brief Initializes the connection manager by setting initial values for connection list, active count, running port, and mutex.

This function initializes the connection manager with default values, including setting the head of the connection list to NULL,

active connection count to 0, and running port to 0. Additionally, it initializes a mutex for thread safety and binds the connection

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
static SensorConnection create_sensor_connection(ClientInfoPacket *packet, int sensor_id)
{
    SensorConnection conn = {
        .socket_fd = packet->sock_fd,
        .port = packet->port,
        .sensor_id = sensor_id,
        .is_active = true,
        .status = CONN_STATUS_CONNECTED,
        .connected_time = time(NULL),
        .last_active_time = time(NULL),
        // .secure_communiation = secure_comm,
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

    // update to connaction manager
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

    printf("client fd by accept client: %d\n", client_fd);

    // for security
    // SecureCommunication *secure_comm = create_secure_connection(client_fd, SECURE_SSL_SERVER);
    // if (secure_comm == NULL)
    // {
    //     close(client_fd);
    //     return;
    // }

    ClientInfoPacket packet;

    if (!read_client_info(client_fd, &packet))
        return;
    printf("client fd by receive client packet: %d", packet.sock_fd);

    // read packet client info by SLL security
    // if (secure_comm->interface.recv(secure_comm->impl, (char *)&packet, sizeof(packet)) <= 0)
    // {
    //     handle_error("Failed to read client info securely");
    //     destroy_secure_connection(secure_comm);
    //     close(client_fd);
    //     return;
    // }

    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    warn_if_ip_mismatch(client_ip, packet.ip_address);

    if (is_port_already_connected((packet.port)))
    {
        handle_error("Port already connected");
        // destroy_secure_connection(secure_comm);
        close(client_fd);
        return;
    }

    int sensor_id = system_manager.connection_manager.active_count;

    // SensorConnection conn = create_sensor_connection(&packet, sensor_id, secure_comm);
    SensorConnection conn = create_sensor_connection(&packet, sensor_id);
    SensorData init_data = create_initial_sensor_data(sensor_id);

    system_manager.connection_manager.add(&system_manager.connection_manager.head, conn, init_data);

    // write to log
    char msg[256];
    snprintf(msg, sizeof(msg), "A sensor node with %d has opened a new connection", sensor_id);
    system_manager.log_manager.log(&system_manager.log_manager, LOG_INFO, "Connection", msg);

    if (!add_client_fd_to_epoll(epoll_fd, client_fd))
    {
        system_manager.connection_manager.remove(&system_manager.connection_manager.head, sensor_id);

        char msg[256];
        snprintf(msg, sizeof(msg), "A sensor node with %d has closed the connection", sensor_id);
        system_manager.log_manager.log(&system_manager.log_manager, LOG_INFO, "Connection", msg);
        // destroy_secure_connection(secure_comm);
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
void *sensor_data_update_thread(void *arg)
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
void check_and_cleanup_inactive_connections()
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
            // Gỡ bỏ kết nối
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

\brief Manages the connection setup and handling for the server.

\details This function initializes the server socket, listens for incoming connections, and handles new connections using epoll. It also tracks the active port.

\param arg Pointer to the port number on which to bind the server.

\return NULL */

void *connection_manager(void *arg)
{
    int port = *(int *)arg;

    int server_fd = create_and_bind_socket(port);

    // save port running
    pthread_mutex_lock(&system_manager.connection_manager.mutex);
    system_manager.connection_manager.running_port = port;
    pthread_mutex_unlock(&system_manager.connection_manager.mutex);

    if (listen(server_fd, 10) < 0)
    {
        perror("Error listen socket");
        exit(EXIT_FAILURE);
    }

    // create epoll
    int epoll_fd = setup_epoll(server_fd);
    struct epoll_event events[10];
    while (1)
    {
        int nfds = epoll_wait(epoll_fd, events, 10, -1);
        if (nfds == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int n = 0; n < nfds; ++n)
        {
            if (events[n].data.fd == server_fd)
            {
                // handle new connection
                handle_new_connection(epoll_fd, server_fd);
            }
            else // handle data from new connection
            {
            }
        }
    }
    close(server_fd);
    return NULL;
}