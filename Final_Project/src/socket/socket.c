#include "socket.h"

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

int create_and_bind_socket(const int port)
{
    int server_fd;
    struct sockaddr_in server_addr;

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

/*---------------------------------------------------------*/
int accept_client(int server_fd, struct sockaddr_in *client_addr, socklen_t *client_len)
{
    int client_fd = accept(server_fd, (struct sockaddr *)client_addr, client_len);
    if (client_fd < 0)
    {
        perror("accept failed");
    }

    return client_fd;
}

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

void warn_if_ip_mismatch(const char *actual_ip, const char *reported_ip)
{
    if (strcmp(actual_ip, reported_ip) != 0)
    {
        printf("Warning: Client info mismatch (reported %s, actual %s)\n", reported_ip, actual_ip);
    }
}
bool add_client_fd_to_epoll(int epoll_fd, int client_fd)
{
    struct epoll_event event;
    memset(&event, 0, sizeof(struct epoll_event));

    event.events = EPOLLIN | EPOLLET;
    event.data.fd = client_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1)
    {
        perror("epoll_ctl add client");
        return false;
    }
    return true;
}

/*--------------------------------------------------------------------------*/
static int create_client_socket()
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        perror("Socket creation failed");
    }
    return sock_fd;
}

static bool connect_to_server(int sock_fd, const char *ip, int port)
{
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid server IP address");
        return false;
    }

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        return false;
    }

    return true;
}

static int get_client_socket_info(int sock_fd, char *ip_buffer)
{
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    if (getsockname(sock_fd, (struct sockaddr *)&client_addr, &len) < 0)
    {
        perror("getsockname failed");
        return -1;
    }

    inet_ntop(AF_INET, &client_addr.sin_addr, ip_buffer, INET_ADDRSTRLEN);
    return ntohs(client_addr.sin_port);
}

static bool send_client_info(int sock_fd, const char *client_ip, int client_port)
{
    ClientInfoPacket info;
    info.sock_fd = sock_fd; //
    strncpy(info.ip_address, client_ip, INET_ADDRSTRLEN);
    // info.port = client_port;
    info.port = system_manager.connection_manager.running_port;

    ssize_t bytes_sent = write(sock_fd, &info, sizeof(info));
    if (bytes_sent != sizeof(info))
    {
        perror("Failed to send client info");
        return false;
    }

    return true;
}

void *client_thread_main(void *arg)
{
    ConnectArgs *args = (ConnectArgs *)arg;
    char *server_ip = args->ip_address;
    int server_port = args->port;

    int sock_fd = create_client_socket();
    if (sock_fd < 0)
        return NULL;

    if (!connect_to_server(sock_fd, server_ip, server_port))
    {
        close(sock_fd);
        return NULL;
    }

    // Tạo kết nối bảo mật SSL từ phía client
    // SecureCommunication *secure_comm = create_secure_connection(sock_fd, SECURE_SSL_CLIENT);
    // if (!secure_comm)
    // {
    //     close(sock_fd);
    //     return NULL;
    // }

    char client_ip[INET_ADDRSTRLEN];
    int client_port = get_client_socket_info(sock_fd, client_ip);
    if (client_port < 0)
    {
        // destroy_secure_connection(secure_comm);
        close(sock_fd);
        return NULL;
    }

    if (!send_client_info(sock_fd, client_ip, client_port))
    {
        close(sock_fd);
        return NULL;
    }

    // ClientInfoPacket info;
    // info.sock_fd = sock_fd;
    // strncpy(info.ip_address, client_ip, INET_ADDRSTRLEN);
    // info.port = system_manager.connection_manager.running_port;

    // Gửi dữ liệu thông qua kênh SSL
    // if (secure_comm->interface.send(secure_comm->impl, (const char *)&info, sizeof(info)) <= 0)
    // {
    //     perror("Failed to send client info securely");
    //     destroy_secure_connection(secure_comm);
    //     close(sock_fd);
    //     return NULL;
    // }

    free(server_ip);
    free(args);
    return NULL;
}

/*----------------initiate connection------------------*/
// static void initiate_connection(const char *ip_address, int port_connect, int local_port)
void start_client_connection(char *ip_address, const int port_connect)
{
    ConnectArgs *args = malloc(sizeof(ConnectArgs));
    args->ip_address = strdup(ip_address);
    args->port = port_connect;

    pthread_t client_thread;
    if (pthread_create(&client_thread, NULL, client_thread_main, args) != 0)
    {
        handle_error("Failed to create client thread");
        free(args->ip_address);
        free(args);
    }
    pthread_detach(client_thread);
}
/*----------------------------------------------------*/