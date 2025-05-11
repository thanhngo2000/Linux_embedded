/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "user_interface.h"
/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/
extern volatile sig_atomic_t keep_running;

/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
/**
 * \brief Parses user input into command and arguments
 *
 * Splits input string into command and arguments using space as delimiter
 * \param input Raw user input string
 * \param parsed_cmd Output structure to store parsed command
 * \return true if parsing successful, false on empty/invalid input
 */
bool parse_user_input(const char *input, ParsedUserCommand *parsed_cmd)
{
    if (!input || strlen(input) == 0)
        return false;

    strncpy(parsed_cmd->input, input, MAX_INPUT_BUFFER - 1);
    parsed_cmd->input[MAX_INPUT_BUFFER - 1] = '\0';

    char *token = strtok(parsed_cmd->input, " ");
    if (!token)
        return false;

    strncpy(parsed_cmd->command, token, sizeof(parsed_cmd->command));
    parsed_cmd->command[sizeof(parsed_cmd->command) - 1] = '\0';

    parsed_cmd->argument_count = 0;

    while ((token = strtok(NULL, " ")) != NULL && parsed_cmd->argument_count < MAX_ARGUMENTS)
    {
        strncpy(parsed_cmd->arguments[parsed_cmd->argument_count], token, MAX_INPUT_BUFFER - 1);
        parsed_cmd->arguments[parsed_cmd->argument_count][MAX_INPUT_BUFFER - 1] = '\0';
        parsed_cmd->argument_count++;
    }

    return true;
}
/**
 * \brief Validates argument count for a command
 *
 * Checks if command has expected number of arguments
 * \param ctx Application context
 * \param cmd Parsed command to validate
 * \param expected Expected argument count
 * \param usage_msg Usage message to display on error
 * \return true if argument count matches, false otherwise
 */
static bool validate_argument_count(AppContext *ctx, ParsedUserCommand *cmd, int expected, const char *usage_msg)
{
    if (cmd->argument_count != expected)
    {
        log_msg(LOG_ERR, "Usage: %s", usage_msg);
        return false;
    }
    return true;
}
/**
 * \brief Executes help command
 *
 * Displays available commands and usage information
 * \param ctx Application context
 * \param cmd Parsed command structure
 */
static void execute_help(AppContext *ctx, ParsedUserCommand *cmd)
{
    if (!validate_argument_count(ctx, cmd, NUM_ARGS_CMD_HELP, "help"))
        return;

    print_sync(ctx,
               "***********************************************************************\n"
               "*********************** Chat Application ******************************\n"
               "***********************************************************************\n\n"
               "Use the commands below:\n"
               "1.  help                            : display user interface options\n"
               "2.  myip                            : display IP address of this app\n"
               "3.  myport                          : display listening port of this app\n"
               "4.  connect <destination> <port no> : connect to the app of another computer\n"
               "5.  list                            : list all the connections of this app\n"
               "6.  terminate <connection id>       : terminate a connection\n"
               "7.  send <connection id> <message>  : send a message to a connection\n"
               "8.  exit                            : close all connections and terminate this app\n"
               "************************************************************************\n");
}
/**
 * \brief Executes myip command
 *
 * Displays local IP address of the application
 * \param ctx Application context
 * \param cmd Parsed command structure
 */
static void execute_myip(AppContext *ctx, ParsedUserCommand *cmd)
{
    if (!validate_argument_count(ctx, cmd, NUM_ARGS_CMD_MYIP, "myip"))
        return;
    log_msg(LOG_INFO, "My IP: %s", ctx->local_info.ip_address);
}
/**
 * \brief Executes myport command
 *
 * Displays listening port of the application
 * \param ctx Application context
 * \param cmd Parsed command structure
 */
static void execute_myport(AppContext *ctx, ParsedUserCommand *cmd)
{
    if (!validate_argument_count(ctx, cmd, NUM_ARGS_CMD_MYPORT, "myport"))
        return;
    log_msg(LOG_INFO, "My Port: %d", ctx->local_info.port);
}
/**
 * \brief Executes connect command
 *
 * Establishes connection to another chat application instance
 * \param ctx Application context
 * \param cmd Parsed command structure
 */
static void execute_connect(AppContext *ctx, ParsedUserCommand *cmd)
{
    if (!validate_argument_count(ctx, cmd, NUM_ARGS_CMD_CONNECT, "connect <ip> <port>"))
        return;

    const char *ip = cmd->arguments[0];
    uint16_t port = atoi(cmd->arguments[1]);

    if (!is_valid_ip_and_port(ctx, ip, port))
        return;

    int sockfd = establish_client_connection(ip, port);
    if (sockfd < 0)
    {
        log_msg(LOG_ERR, "Failed to connect to %s:%d", ip, port);
        return;
    }

    bool added = add_peer_manager(ctx->peer_manager, ip, port, sockfd);
    if (added)
    {
        log_msg(LOG_INFO, "Connected to %s:%d and added to peer list.", ip, port);

        PeerConnection server_conn = {
            .socket_fd = sockfd,
            .ip_address = "",
            .port = port};

        strncpy(server_conn.ip_address, ip, sizeof(server_conn.ip_address) - 1);

        if (!handle_message_send(ctx, &server_conn, MSG_CONNECTED, "connected"))
        {
            log_msg(LOG_WARN, "Failed to send client info message to server.");
        }

        start_client_receive_thread(ctx, sockfd);
    }
    else
    {
        log_msg(LOG_WARN, "Peer list full. Connection to %s:%d rejected.", ip, port);
        close(sockfd);
    }
}
/**
 * \brief Executes list command
 *
 * Displays all active connections
 * \param ctx Application context
 * \param cmd Parsed command structure
 */
static void execute_list(AppContext *ctx, ParsedUserCommand *cmd)
{
    if (!validate_argument_count(ctx, cmd, NUM_ARGS_CMD_LIST, "list"))
        return;

    pthread_mutex_lock(&ctx->peer_manager->connection_mutex);

    print_sync(ctx, "\n📋 Active Connections:\n");
    print_sync(ctx, "ID\t\tIP Address\t\tPort\t\tSocket FD\n");
    print_sync(ctx, "---------------------------------------------------------------\n");

    for (int i = 0; i < MAX_PEERS; ++i)
    {
        PeerConnection *p = &ctx->peer_manager->peers[i];
        if (p->active)
        {
            print_sync(ctx, "%d\t\t%s\t\t%d\t\t%d\n", p->id, p->ip_address, p->port, p->socket_fd);
        }
    }

    print_sync(ctx, "---------------------------------------------------------------\n\n");

    pthread_mutex_unlock(&ctx->peer_manager->connection_mutex);
}
/**
 * \brief Executes send command
 *
 * Sends message to specified peer connection
 * \param ctx Application context
 * \param cmd Parsed command structure
 */
static void execute_send(AppContext *ctx, ParsedUserCommand *cmd)
{
    if (cmd->argument_count < 2)
    {
        log_msg(LOG_ERR, "Usage: send <peer_index> <message>");
        return;
    }

    const int index = atoi(cmd->arguments[0]);

    char message[MAX_MESSAGE_LENGTH + 1] = {0};
    size_t offset = 0;
    for (int i = 1; i < cmd->argument_count; ++i)
    {
        size_t remaining = MAX_MESSAGE_LENGTH - offset;
        strncat(message, cmd->arguments[i], remaining);
        offset = strlen(message);

        if (i < cmd->argument_count - 1 && offset < MAX_MESSAGE_LENGTH)
        {
            strncat(message, " ", MAX_MESSAGE_LENGTH - offset);
            offset++;
        }
    }
    log_msg(LOG_INFO, "Buffer message: %s", message);

    PeerConnection *receiver = get_peer_by_index(ctx->peer_manager, index);
    if (!receiver)
    {
        log_msg(LOG_ERR, "Invalid peer index or peer is not active.");
        return;
    }

    if (!handle_message_send(ctx, receiver, MSG_SEND, message))
    {
        log_msg(LOG_ERR, "Failed to send message to [%d] (%s:%d)",
                index, receiver->ip_address, receiver->port);
        return;
    }

    log_msg(LOG_INFO, "Sent to [%d]: %s", index, message);
}
/**
 * \brief Executes terminate command
 *
 * Terminates specified peer connection
 * \param ctx Application context
 * \param cmd Parsed command structure
 */
static void execute_terminate(AppContext *ctx, ParsedUserCommand *cmd)
{
    if (!validate_argument_count(ctx, cmd, NUM_ARGS_CMD_TERMINATE, "terminate <device_id>"))
        return;

    int id = atoi(cmd->arguments[0]);
    if (id < 0 || id >= MAX_PEERS)
    {
        log_msg(LOG_ERR, "Invalid device ID: %s", cmd->arguments[0]);
        return;
    }

    PeerConnection *peer = get_peer_by_index(ctx->peer_manager, id);
    if (!peer || !peer->active)
    {
        log_msg(LOG_WARN, "Could not find active connection with device ID %d.", id);
        return;
    }

    const char *ip = peer->ip_address;
    uint16_t port = peer->port;
    int sockfd = peer->socket_fd;

    if (!handle_message_send(ctx, peer, MSG_TERMINATED, ""))
    {
        log_msg(LOG_WARN, "Failed to send terminated message to server.");
    }
    else
    {
        log_msg(LOG_INFO, "Terminated message sent to server: %s:%d (socket %d)", ip, port, sockfd);
    }

    if (remove_peer_by_id(ctx->peer_manager, id))
    {
        log_msg(LOG_INFO, "Terminated connection with device ID %d.", id);
    }
    else
    {
        log_msg(LOG_WARN, "Failed to remove peer with ID %d.", id);
    }
}
/**
 * \brief Executes exit command
 *
 * Closes all connections and terminates application
 * \param ctx Application context
 * \param cmd Parsed command structure
 */
static void execute_exit(AppContext *ctx, ParsedUserCommand *cmd)
{
    if (!validate_argument_count(ctx, cmd, NUM_ARGS_CMD_EXIT, "exit"))
        return;

    log_msg(LOG_INFO, "Exiting...");
    if (ctx && ctx->peer_manager)
    {
        int count = count_peer_manager(ctx->peer_manager);
        if (count > 0)
        {
            char msg[128];
            snprintf(msg, sizeof(msg), "🔌 Closing %d peer connection(s)...\n", count);
            log_msg(LOG_INFO, "%s", msg);
        }

        close_all_peer_manager(ctx->peer_manager);
    }
    keep_running = 0;
}
/**
 * \brief Command dispatch table
 *
 * Maps command names to their respective handler functions
 */
static CommandEntry command_table[] = {
    {"help", execute_help},
    {"myip", execute_myip},
    {"myport", execute_myport},
    {"connect", execute_connect},
    {"list", execute_list},
    {"send", execute_send},
    {"terminate", execute_terminate},
    {"exit", execute_exit},
    {NULL, NULL}};
/**
 * \brief Finds command handler for given command
 *
 * Looks up command in dispatch table and returns handler
 * \param cmd Parsed command to dispatch
 * \return Pointer to CommandEntry if found, NULL otherwise
 */
CommandEntry *dispatch_command(const ParsedUserCommand *cmd)
{
    static CommandEntry command;
    for (int i = 0; command_table[i].name != NULL; ++i)
    {
        if (strcmp(command_table[i].name, cmd->command) == 0)
        {
            command.execute = command_table[i].execute;
            command.name = command_table[i].name;
            return &command;
        }
    }
    return NULL;
}