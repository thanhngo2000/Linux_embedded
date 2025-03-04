/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "user_interface.h"
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
 * @brief Prints the user interface menu for the chat application.
 *
 * This function displays a list of commands that can be used by the user to interact with the chat application.
 * The menu includes options such as displaying help, displaying IP address, displaying listening port, connecting to another computer,
 * listing all connections, terminating a connection, sending a message to a connection, and closing all connections and terminating the app.
 *
 * @return void
 */
void print_menu()
{
    printf("***********************************************************************\n");
    printf("*********************** Chat Application ****************************\n");
    printf("********************************************************************\n\n");
    printf("Use the commands below:\n\n");
    printf("1.  help                            : display user interface options\n");
    printf("2.  myip                            : display IP address of this app\n");
    printf("3.  myport                          : display listening port of this app\n");
    printf("4.  connect <destination> <port no> : connect to the app of another computer\n");
    printf("5   list                            : list all the connections of this app\n");
    printf("6.  terminate <connection id>       : terminate a connection\n");
    printf("7.  send <connection id> <message>  : send a message to a connection\n");
    printf("8.  close all connections           : close all connections and terminate this app\n");
    printf("************************************************************************\n");
}

/**
 * @brief Handles user commands for the chat application.
 *
 * This function processes user commands and performs the corresponding actions.
 * The supported commands include displaying help, displaying IP address, displaying listening port, connecting to another computer,
 * listing all connections, terminating a connection, sending a message to a connection, and closing all connections and terminating the app.
 *
 * @param peer A pointer to the PeerInfo structure representing the current peer.
 * @param command A string containing the user command.
 *
 * @return void
 */
void CommandHandler_handleCommand(PeerInfo *peer, const char *command)
{
    // Implementation...
    if (strncmp("help", command, 4) == 0)
    {
        if (!check_user_input(command, NUM_CMD_HELP))
        {
            handle_error("Error: command help");
        }
        else
        {
            print_menu();
        }
    }
    else if (strncmp("myip", command, 4) == 0)
    {
        if (!check_user_input(command, NUM_CMD_MYIP))
        {
            handle_error("Error: command myip");
        }
        else
        {
            printf("Display ip\n");
            printf("My ip is %s\n", peer->ip_address);
        }
    }
    else if (strncmp("myport", command, 6) == 0)
    {
        if (!check_user_input(command, NUM_CMD_MYPORT))
        {
            handle_error("Error: command myport");
        }
        else
        {
            printf("Display my port\n");
            printf("My port is %d\n", peer->port);
        }
    }
    else if (strncmp("connect", command, 7) == 0)
    {
        if (!check_user_input(command, NUM_CMD_CONNECT))
        {
            handle_error("Error: command connect");
        }
        else
        {
            printf("Connect to another computer\n");
            cmd_connect_handler(command);
        }
    }
    else if (strncmp("list", command, 4) == 0)
    {
        if (!check_user_input(command, NUM_CMD_LIST))
        {
            handle_error("Error: command list");
        }
        else
        {
            printf("List all connections\n");
            display_connections();
        }
    }
    else if (strncmp("terminate", command, 9) == 0)
    {
        if (!check_user_input(command, NUM_CMD_TERMINATE))
        {
            handle_error("Error: command terminate");
        }
        else
        {
            printf("Terminate connection\n");
            cmd_terminate_handler(command);
        }
    }
    else if (strncmp("send", command, 4) == 0)
    {
        printf("Send message\n");
        cmd_send_handler(command);
    }
    else if (strncmp("exit", command, 4) == 0)
    {
        if (!check_user_input(command, NUM_CMD_EXIT))
        {
            handle_error("Error: command exit");
        }
        else
        {
            printf("Close connection and terminate\n");
            cmd_exit_handler(command);
        }
    }
    else
    {
        printf("Unknown command\n");
    }
}

/**
 * @brief Handles the 'connect' command by parsing the command string, validating the IP address and port number,
 * and initiating a socket client connection to the specified IP address and port.
 *
 * @param command A string containing the 'connect' command. The command should be in the format: "connect <ip> <port>".
 *
 * @return void
 *
 */
void cmd_connect_handler(const char *command)
{
    char info_connect[MAX_WORDS][MAX_WORD_LENGTH];

    split_string(command, info_connect, NUM_CMD_CONNECT);
    // for (int i = 0; i < NUM_CMD_CONNECT; i++)
    // {
    //     printf("%s\n", info_connect[i]); // connect <ip> <port>
    // }

    // save port info connection
    int port_connect = atoi(info_connect[2]);
    char ip_connect[INET_ADDRSTRLEN];
    strcpy(ip_connect, info_connect[1]);

    if (!is_valid_ip(ip_connect))
    {
        handle_error("Error: Invalid IP address");
    }
    else if (!is_valid_port(port_connect))
    {
        handle_error("Error: Invalid port number. Must be between 1 and 65535");
    }
    else if (is_port_in_list(port_connect) == true)
    {
        handle_error("Error: Port is already in list");
    }
    else if (is_same_port(port_connect) == true)
    {
        handle_error("Error: same port running");
    }
    else // run socket client
    {
        run_client(ip_connect, port_connect);
    }
}

/**
 * @brief Handles the 'terminate' command by parsing the command string, validating the connection ID,
 * and sending a termination message to the specified connection.
 *
 * @param command A string containing the 'terminate' command. The command should be in the format: "terminate <id>".
 *
 * @return void
 */
void cmd_terminate_handler(const char *command)
{
    char info_terminate[MAX_WORDS][MAX_WORD_LENGTH];

    split_string(command, info_terminate, NUM_CMD_TERMINATE);
    // for (int i = 0; i < NUM_CMD_TERMINATE; i++)
    // {
    //     printf("%s\n", info_terminate[i]);
    // }

    if (is_id_in_list(atoi(info_terminate[1])) == false)
    {
        handle_error("Error: Invalid connection id");
    }
    else
    {
        int id = atoi(info_terminate[1]) - 1; // client_info start with 0

        // send termination port info to server
        char message[BUFFER_SIZE];
        sprintf(message, "terminated: %d\n", peer.port);
        if (send(peer.active_connections[id].socket_fd, message, sizeof(message), 0) < 0)
        {
            handle_error("Failed to send termination port");
        }

        // find port by id
        remove_connection(peer.active_connections[id].port);
    }
}

/**
 * @brief Handles the 'send' command by parsing the command string, validating the connection ID,
 * and sending a message to the specified connection.
 *
 * @param command A string containing the 'send' command. The command should be in the format: "send <id> <message>".
 *
 * @return void
 *
 */
void cmd_send_handler(const char *command)
{
    char info_send[MAX_WORDS][MAX_WORD_LENGTH];

    split_string(command, info_send, NUM_CMD_SEND); // send <id> <message>
    // for (int i = 0; i < NUM_CMD_SEND; i++)
    // {
    //     printf("%s\n", info_send[i]);
    // }

    int id = atoi(info_send[1]) - 1; // list start with index 0
    char message_receive[BUFFER_SIZE];
    strcpy(message_receive, info_send[2]);

    if (is_id_in_list(atoi(info_send[1])) == false)
    {
        handle_error("Error: Invalid connection id");
    }
    else
    {
        char message[BUFFER_SIZE];

        sprintf(message, "send %s %d %s\n", peer.ip_address, peer.port, message_receive);
        if (send(peer.active_connections[id].socket_fd, message, sizeof(message), 0) < 0)
        {
            handle_error("Failed to send message in cmd send");
        }
        // printf("Message sent: %s\n", message);
    }
}

/**
 * @brief Handles the 'exit' command by sending termination messages to all active connections,
 * closing all connections, and terminating the server.
 *
 * @param command A string containing the 'exit' command. The command should be in the format: "exit".
 *
 * @return void
 */
void cmd_exit_handler(const char *command)
{
    printf("Currrent port: %d\n", peer.port);
    // close connections and terminate server
    for (int i = 0; i < peer.active_connection_count; i++)
    {
        char message[BUFFER_SIZE];

        sprintf(message, "exit %d\n", peer.port);
        if (send(peer.active_connections[i].socket_fd, message, sizeof(message), 0) < 0)
        {
            handle_error("Failed to send message in cmd exit");
        }
        // printf("Message sent: %s\n", message);
    }
    pthread_mutex_destroy(&lock);
    close(peer.server_socket_fd);
    exit(EXIT_SUCCESS);
}
