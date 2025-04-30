#include "utils.h"
/**
 * \brief Prints an error message to stderr.
 *
 * \param message The error message to be printed.
 *
 * \note This function prints the provided error message to the standard error stream.
 */
void handle_error(const char *message)
{
    fprintf(stderr, "Error: %s\n", message);
}
/**
 * \brief Checks if the provided IP address is valid.
 *
 * \param ip The IP address string to be validated.
 *
 * \return true if the IP address is valid, false otherwise.
 *
 * \note This function checks if the IP address is not NULL or empty,
 *       ensures it does not exceed the maximum length, and verifies its format using inet_pton.
 */
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

/**
 * \brief Checks if the provided port number is valid.
 *
 * \param port The port number to be validated.
 *
 * \return true if the port is between 1 and 65535, false otherwise.
 *
 * \note This function ensures the port number is within the valid range for ports.
 */
bool is_valid_port(const int port)
{
    return port > 0 && port <= 65535;
}
/**
 * \brief Checks if the given port matches the running port.
 *
 * \param port The port to check.
 * \param running_port The current running port to compare against.
 *
 * \return true if the given port is the same as the running port, false otherwise.
 */
bool is_running_port(const int port, const int running_port)
{
    return port == running_port; //
}
/**
 * \brief Checks if the given port is already in use by an existing connection.
 *
 * \param port The port to check.
 *
 * \return true if the port is already connected, false otherwise.
 *
 * \note This function checks the connection list and uses a mutex to ensure thread safety during the check.
 */
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
/**
 * \brief Validates the connection parameters (IP and port).
 *
 * \param ip The IP address for the connection.
 * \param port The port number for the connection.
 * \param running_port The currently running port to avoid conflicts.
 *
 * \return 1 if the parameters are valid, 0 if any of the parameters are invalid.
 *
 * \note This function checks the validity of the IP address and port number, and ensures that the port is not already in use.
 */
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