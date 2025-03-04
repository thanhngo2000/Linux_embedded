/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "utils.h"

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/

/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/

/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/

/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/

/**
 * @brief Prints an error message to the standard error stream.
 *
 * @param message A pointer to a null-terminated string containing the error message to be printed.
 *
 * @return This function does not return a value.
 */
void handle_error(const char *message)
{
    fprintf(stderr, "Error: %s\n", message);
}

/**
 * @brief Checks if the number of words in the input string matches the desired count.
 *
 *
 * @param input A pointer to a null-terminated string containing the input to be checked.
 * @param desired_count The expected number of words in the input string.
 *
 * @return true if the number of words in the input string matches the desired count, false otherwise.
 */
int check_user_input(const char *input, const int desired_count)
{
    int count = 0;
    const char *token = strtok(strdup(input), " "); // create copy

    while (token != NULL)
    {
        count++;
        token = strtok(NULL, " ");
    }

    return count == desired_count; // return true if number strings were true
}

/**
 * @brief Splits a string into an array of words based on whitespace.
 *
 * @param input A pointer to a null-terminated string containing the input to be split.
 * @param words A 2D array of characters to store the resulting words.
 * @param desired_count The maximum number of words to store in the array.
 *
 * @return This function does not return a value.
 */
void split_string(const char *input, char words[MAX_WORDS][MAX_WORD_LENGTH], int desired_count)
{
    char *token;
    char *input_copy = strdup(input);

    int word_count = 0;
    token = strtok(input_copy, " ");
    while (token != NULL && word_count < MAX_WORDS)
    {
        strncpy(words[word_count], token, MAX_WORD_LENGTH);
        words[word_count][MAX_WORD_LENGTH - 1] = '\0';
        word_count++;
        token = strtok(NULL, " ");
    }

    if (word_count > desired_count)
    {
        for (int i = desired_count; i < word_count; i++)
        {
            strncat(words[desired_count - 1], " ", 1);
            strncat(words[desired_count - 1], words[i], MAX_WORD_LENGTH - strlen(words[desired_count - 1]) - 1);
        }
        word_count = desired_count;
    }

    free(input_copy);
}

/**
 * @brief Retrieves the local IP address of the device.
 *
 * @return A pointer to a statically allocated string containing the local IP address. If no suitable IP address is found,
 *         the function returns NULL.
 */
char *get_ip()
{
    struct ifaddrs *ifaddr, *ifa;
    static char ip_str[INET_ADDRSTRLEN];

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
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
 * @brief Checks if a given port number is already in the list of active connections.
 *
 * @param port_connect The port number to be checked.
 *
 * @return true if the given port number is already in the list of active connections, false otherwise.
 */
bool is_port_in_list(const int port_connect)
{
    for (int i = 0; i < peer.active_connection_count; i++)
    {
        if (port_connect == peer.active_connections[i].port)
        {
            return true; // conflict with port in list
        }
    }
    return false;
}

/**
 * @brief Checks if a given port number is the same as the local peer's port.
 *
 *
 * @param port The port number to be checked.
 *
 * @return true if the given port number is the same as the local peer's port, false otherwise.
 */
bool is_same_port(const int port)
{
    if (port == peer.port)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief Checks if a given IP address is valid.
 *
 *
 * @param ip A pointer to a null-terminated string containing the IP address to be checked.
 *
 * @return true if the given IP address is valid, false otherwise.
 */
bool is_valid_ip(const char *ip)
{
    struct sockaddr_in sa;
    if (inet_pton(AF_INET, ip, &(sa.sin_addr)) != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief Checks if a given port number is valid.
 *
 * @param port The port number to be checked.
 *
 * @return true if the given port number is valid (i.e., falls within the range 1 to 65535), false otherwise.
 */
bool is_valid_port(const int port)
{
    if (port > 0 && port <= 65535)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief Checks if a given ID is already in the list of active connections.
 *
 *
 * @param id The ID to be checked.
 *
 * @return true if the given ID is already in the list of active connections, false otherwise.
 */
bool is_id_in_list(const int id)
{
    for (int i = 0; i < peer.active_connection_count; i++)
    {
        if (id == peer.active_connections[i].id)
        {
            return true; // id in list
        }
    }
    return false;
}
