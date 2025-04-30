/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "socket.h"
/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
/**
 * \brief Sets up the epoll instance and adds the server socket to the epoll set for monitoring incoming events.
 *
 * \param server_fd The file descriptor for the server socket.
 *
 * \return int The epoll file descriptor that can be used to wait for events.
 *
 * \note This function initializes an epoll instance using epoll_create1, then adds the server socket to the epoll set.
 * It ensures proper error handling in case of failure.
 */
int setup_epoll(int server_fd)
{
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1)
    {
        perror("epoll_ctl: server_fd");
        exit(EXIT_FAILURE);
    }

    return epoll_fd;
}
/**
 * \brief Creates and binds a socket to the specified port.
 *
 * \param port The port number to bind the server socket to.
 *
 * \return int The file descriptor of the server socket.
 *
 * \note This function creates a TCP socket, sets up the server address, and binds it to the specified port.
 * It handles errors and terminates the process if any operation fails.
 */
int create_and_bind_socket(const int port)
{
    int server_fd;
    struct sockaddr_in server_addr = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

/**
 * \brief Accepts a new incoming client connection on the server socket.
 *
 * \param server_fd The file descriptor of the server socket.
 * \param client_addr A pointer to a sockaddr_in structure to store the client's address information.
 * \param client_len A pointer to the length of the client's address structure.
 *
 * \return int The file descriptor of the accepted client socket.
 *
 * \note This function accepts a client connection and returns the file descriptor for the client.
 * It reports an error if the accept operation fails.
 */
int accept_client(int server_fd, struct sockaddr_in *client_addr, socklen_t *client_len)
{
    int client_fd = accept(server_fd, (struct sockaddr *)client_addr, client_len);
    if (client_fd < 0)
    {
        perror("accept failed");
    }

    return client_fd;
}
/**
 * \brief Reads client information from the client socket and stores it in the provided packet.
 *
 * \param client_fd The file descriptor of the client socket.
 * \param packet A pointer to a ClientInfoPacket structure to store the client's information.
 *
 * \return bool Returns true if the client information was successfully read, false otherwise.
 *
 * \note This function reads data from the client socket and verifies the number of bytes read matches
 * the expected packet size. If there is an issue, the client connection is closed.
 */
bool read_client_info(int client_fd, ClientInfoPacket *packet)
{
    ssize_t bytes_read = read(client_fd, packet, sizeof(*packet));
    if (bytes_read != sizeof(*packet))
    {
        perror("Failed to read client info");
        close(client_fd);
        return false;
    }
    return true;
}

/**
 * \brief Warns if there is a mismatch between the actual IP and the reported IP.
 *
 * \param actual_ip The actual IP address obtained from the client.
 * \param reported_ip The IP address reported by the client.
 *
 * \note This function compares the two IP addresses and prints a warning message if they do not match.
 */
void warn_if_ip_mismatch(const char *actual_ip, const char *reported_ip)
{
    if (strcmp(actual_ip, reported_ip) != 0)
    {
        printf("Warning: Client info mismatch (reported %s, actual %s)\n", reported_ip, actual_ip);
    }
}
/**
 * \brief Adds the client file descriptor to the epoll instance for monitoring.
 *
 * \param epoll_fd The epoll file descriptor.
 * \param client_fd The file descriptor of the client socket to be added to the epoll set.
 *
 * \return bool Returns true if the client socket was successfully added, false otherwise.
 *
 * \note This function creates an epoll event for the client socket and adds it to the epoll set.
 * It sets the client socket to edge-triggered mode (EPOLLET) to handle events efficiently.
 */
bool add_client_fd_to_epoll(int epoll_fd, int client_fd)
{
    struct epoll_event event;
    memset(&event, 0, sizeof(struct epoll_event));

    event.events = EPOLLIN | EPOLLET;
    event.data.fd = client_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1)
    {
        handle_error("epoll_ctl add client");
        return false;
    }
    return true;
}

/**
 * \brief Creates a new socket.
 *
 * \return int The file descriptor of the newly created socket.
 *
 * \note This function creates a TCP socket using the socket() system call.
 * It handles errors if socket creation fails.
 */
int create_socket()
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        handle_error("Socket creation failed");
    }
    return sock_fd;
}
/**
 * \brief Connects the client socket to the specified server using the provided IP and port.
 *
 * \param sock_fd The file descriptor of the client socket.
 * \param ip The IP address of the server to connect to.
 * \param port The port number of the server to connect to.
 *
 * \return bool Returns true if the connection is successfully established, false otherwise.
 *
 * \note This function configures the server address and attempts to establish a TCP connection.
 * It handles errors if the connection attempt fails.
 */
static bool connect_to_server(int sock_fd, const char *ip, int port)
{
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
    {
        handle_error("Invalid server IP address");
        return false;
    }

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        handle_error("Connection failed");
        return false;
    }

    return true;
}
/**
 * \brief Retrieves the client's socket port, including IP address and port.
 *
 * \param sock_fd The file descriptor of the client socket.
 * \param ip_buffer A buffer to store the client's IP address.
 *
 * \return int The client's port number on success, -1 on failure.
 *
 * \note This function uses getsockname() to obtain the client's local address and port number.
 * It converts the address to a string and returns the client's port number.
 */
static int get_port(int sock_fd, char *ip_buffer)
{
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    if (getsockname(sock_fd, (struct sockaddr *)&client_addr, &len) < 0)
    {
        handle_error("getsockname failed");
        return -1;
    }

    inet_ntop(AF_INET, &client_addr.sin_addr, ip_buffer, INET_ADDRSTRLEN);
    return ntohs(client_addr.sin_port);
}
/**
 * \brief Sends client information, such as socket file descriptor, IP address, and port, to the server.
 *
 * \param sock_fd The file descriptor of the client socket.
 * \param client_ip The IP address of the client.
 * \param client_port The port number of the client.
 *
 * \return bool Returns true if the information was successfully sent, false otherwise.
 *
 * \note This function creates a ClientInfoPacket structure, populates it with the client information,
 * and sends it to the server via the socket. It handles errors if sending the data fails.
 */
// static bool send_client_info(int sock_fd, const char *client_ip, int client_port)
// {
//     ClientInfoPacket info;
//     info.sock_fd = sock_fd; //
//     strncpy(info.ip_address, client_ip, INET_ADDRSTRLEN);
//     // info.port = client_port;
//     info.port = system_manager.connection_manager.running_port;

//     ssize_t bytes_sent = write(sock_fd, &info, sizeof(info));
//     if (bytes_sent != sizeof(info))
//     {
//         handle_error("Failed to send client info");
//         return false;
//     }

//     return true;
// }
/**
 * \brief The main function for the client thread that connects to a server, sends client information, and handles communication.
 *
 * \param arg A pointer to the arguments passed to the thread, including server IP and port.
 *
 * \return void* Always returns NULL.
 *
 * \note This function runs in a separate thread, where it connects to the server, retrieves client information,
 * sends it to the server, and manages the socket lifecycle. It handles errors in the process and ensures the socket is properly closed.
 */
void *client_thread_main(void *arg)
{

    ConnectArgs *args = (ConnectArgs *)arg;
    char *server_ip = args->ip_address;
    int server_port = args->port;

    printf("IP server connect: %s\n", (server_ip));
    printf("Server port: %d\n", server_port);

    int sock_fd = create_socket();
    if (sock_fd < 0)
    {
        free(args->ip_address);
        free(args);
        return NULL;
    }

    printf("Create client socket success\n");

    if (!connect_to_server(sock_fd, server_ip, server_port))
    {
        close(sock_fd);
        free(args->ip_address);
        free(args);
        return NULL;
    }

    printf("Connect to server success\n");

    char client_ip[INET_ADDRSTRLEN];

    int client_port = get_port(sock_fd, client_ip);
    if (client_port < 0)
    {
        close(sock_fd);
        free(args->ip_address);
        free(args);
        return NULL;
    }

    // create SSL security from client
    SecureCommunication *secure_comm = create_secure_connection(sock_fd, SECURE_SSL_CLIENT);
    if (!secure_comm)
    {
        close(sock_fd);
        free(args->ip_address);
        free(args);
        return NULL;
    }
    ClientInfoPacket info;
    info.sock_fd = sock_fd;
    strncpy(info.ip_address, client_ip, INET_ADDRSTRLEN);
    info.port = system_manager.connection_manager.running_port;

    // send data with SSL
    if (secure_comm->interface.send(secure_comm->impl, (const char *)&info, sizeof(info)) <= 0)
    {
        handle_error("Failed to send client info securely");
        destroy_secure_connection(secure_comm);
        close(sock_fd);
        return NULL;
    }

    free(server_ip);
    free(args);
    return NULL;
}
/**
 * \brief Starts a client connection by creating a new thread to handle the connection process.
 *
 * \param ip_address The IP address of the server to connect to.
 * \param port_connect The port number of the server to connect to.
 *
 * \note This function allocates memory for the connection arguments, creates a client thread,
 * and starts the client connection process. It ensures proper thread management.
 */
void start_client_connection(char *ip_address, const int port_connect)
{
    ConnectArgs *args = malloc(sizeof(ConnectArgs));
    if (!args)
    {
        handle_error("Failed to allocate memory for connection args");
        return;
    }

    args->ip_address = strdup(ip_address);
    if (!args->ip_address)
    {
        free(args);
        handle_error("Failed to duplicate IP address");
        return;
    }

    args->port = port_connect;

    pthread_t client_thread;
    if (pthread_create(&client_thread, NULL, client_thread_main, args) != 0)
    {
        free(args->ip_address);
        free(args);
        handle_error("Failed to create client thread");
        return;
    }
    pthread_detach(client_thread);
}
