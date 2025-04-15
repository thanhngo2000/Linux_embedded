#include "utils.h"

void handle_error(const char *message)
{
    fprintf(stderr, "Error: %s\n", message);
}

bool is_valid_ip(const char *ip)
{
    // check NULL or empty
    if (ip == NULL || *ip == '\0')
    {
        return false;
    }

    // Check max length IP address
    if (strlen(ip) > INET_ADDRSTRLEN - 1)
    {
        return false;
    }

    struct sockaddr_in sa;

    // check Ipv4 by inet_pton
    int result = inet_pton(AF_INET, ip, &(sa.sin_addr));
    return result;
}

// Hàm kiểm tra port hợp lệ
bool is_valid_port(const int port)
{
    return port > 0 && port <= 65535;
}
bool is_running_port(const int port, const int running_port)
{
    return port == running_port; //
}
bool is_port_already_connected(const int port)
{
    pthread_mutex_lock(&system_manager.connection_manager.mutex);

    ConnectionNode *current = system_manager.connection_manager.head;
    while (current != NULL)
    {
        if (current->connection.port == port &&
            current->connection.status == CONN_STATUS_CONNECTED)
        {
            pthread_mutex_unlock(&system_manager.connection_manager.mutex);
            return true;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&system_manager.connection_manager.mutex);
    return false;
}
int validate_connection_params(char *ip, const int port, const int running_port)
{
    if (!is_valid_port(port))
    {
        handle_error("Invalid port number. Must be between 1 and 65535");
        return 0;
    }
    if (!is_valid_ip(ip))
    {
        handle_error("Invalid ip address");
        return 0;
    }
    if (is_running_port(port, running_port))
    {
        handle_error("EPort already running");
        return 0;
    }

    return 1;
}