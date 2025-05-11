/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "message.h"
/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
/**
 * \brief Splits a string into tokens based on spaces and newlines
 *
 * Splits input string into words while handling buffer overflow protection.
 * Any excess words beyond max_words-1 will be concatenated into the last output slot.
 *
 * \param str Input string to split
 * \param output 2D array to store output words
 * \param max_words Maximum number of words to extract
 * \return Actual number of words extracted (capped at max_words)
 */
static int split_string(const char *str, char output[][MAX_MESSAGE_LENGTH], int max_words)
{
    int count = 0;
    char buffer[MAX_MESSAGE_LENGTH * max_words];
    strncpy(buffer, str, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char *token = strtok(buffer, " \n");

    while (token != NULL)
    {
        if (count < max_words - 1)
        {
            strncpy(output[count], token, MAX_MESSAGE_LENGTH - 1);
            output[count][MAX_MESSAGE_LENGTH - 1] = '\0';
            count++;
        }
        else
        {
            if (count == max_words - 1)
            {
                strncpy(output[count], token, MAX_MESSAGE_LENGTH - 1);
                output[count][MAX_MESSAGE_LENGTH - 1] = '\0';
            }
            else
            {
                // add space
                strncat(output[max_words - 1], " ", MAX_MESSAGE_LENGTH - strlen(output[max_words - 1]) - 1);
                strncat(output[max_words - 1], token, MAX_MESSAGE_LENGTH - strlen(output[max_words - 1]) - 1);
            }
            count++;
        }

        token = strtok(NULL, " \n");
    }

    // check data > max_words?
    return (count > max_words) ? max_words : count;
}
/**
 * \brief Handles CONNECTED message type
 *
 * Processes peer connection request and adds to peer manager if valid.
 * Logs success/failure of peer addition.
 *
 * \param ctx Application context
 * \param sockfd Socket file descriptor
 * \param words Parsed message tokens
 * \param count Number of valid tokens
 */
static void handle_connected(AppContext *ctx, int sockfd, char words[][MAX_MESSAGE_LENGTH], int count)
{
    if (count >= 3)
    {
        const char *ip = words[1];
        uint16_t port = (uint16_t)atoi(words[2]);

        if (ctx->peer_manager)
        {
            if (add_peer_manager(ctx->peer_manager, ip, port, sockfd))
            {
                log_msg(LOG_INFO, "\n✅ Peer added: %s:%d (socket %d)\n", ip, port, sockfd);
            }
            else
            {
                log_msg(LOG_ERR, "\n❌ Failed to add peer %s:%d (socket %d)\n", ip, port, sockfd);
            }
        }
    }
}
/**
 * \brief Handles TERMINATED message type
 *
 * Processes peer termination request and removes from peer manager.
 * Logs success/failure of peer removal.
 *
 * \param ctx Application context
 * \param sockfd Socket file descriptor
 * \param words Parsed message tokens
 * \param count Number of valid tokens
 */
static void handle_terminated(AppContext *ctx, int sockfd, char words[][MAX_MESSAGE_LENGTH], int count)
{
    if (count >= 2)
    {
        const char *terminated_ip = words[1];

        if (ctx && ctx->peer_manager)
        {
            bool removed = remove_peer_by_socket(ctx->peer_manager, sockfd);
            if (removed)
            {
                log_msg(LOG_INFO, "\n✅ Peer %s (socket %d) removed successfully.\n", terminated_ip, sockfd);
            }
            else
            {
                log_msg(LOG_ERR, "\n❌ Failed to remove peer %s (socket %d)\n", terminated_ip, sockfd);
            }
        }
    }
}
/**
 * \brief Handles SEND message type
 *
 * Processes incoming message and logs sender info + message content.
 *
 * \param ctx Application context
 * \param sockfd Socket file descriptor
 * \param words Parsed message tokens
 * \param count Number of valid tokens
 */
static void handle_send(AppContext *ctx, int sockfd, char words[][MAX_MESSAGE_LENGTH], int count)
{
    if (count >= 4)
    {
        log_msg(LOG_INFO, "\nMessage received from %s", words[1]);
        log_msg(LOG_INFO, "Sender's Port: %s", words[2]);
        log_msg(LOG_INFO, "Message: %s", words[3]);
    }
}
/**
 * \brief Handles EXIT message type
 *
 * Processes exit notification from peer and logs the event.
 *
 * \param ctx Application context
 * \param sockfd Socket file descriptor
 * \param words Parsed message tokens
 * \param count Number of valid tokens
 */
static void handle_exit(AppContext *ctx, int sockfd, char words[][MAX_MESSAGE_LENGTH], int count)
{
    if (count >= 2)
        log_msg(LOG_INFO, "Exit message received from %s", words[1]);
}
/**
 * \brief Message handler strategies lookup table
 *
 * Maps message types to their respective handler functions
 * and expected argument counts.
 */
static MessageHandlerStrategy strategies[] = {
    {MSG_CONNECTED, 4, handle_connected},
    {MSG_TERMINATED, 3, handle_terminated},
    {MSG_SEND, 4, handle_send},
    {MSG_EXIT, 3, handle_exit},
};
/**
 * \brief Determines message type from string command
 *
 * \param msg_type Message type string to identify
 * \return Corresponding MessageType enum value
 */
static MessageType get_message_type(const char *msg_type)
{
    if (strcmp(msg_type, "connect") == 0)
        return MSG_CONNECTED;
    if (strcmp(msg_type, "terminate") == 0)
        return MSG_TERMINATED;
    if (strcmp(msg_type, "send") == 0)
        return MSG_SEND;
    if (strcmp(msg_type, "exit") == 0)
        return MSG_EXIT;
    return MSG_UNKNOWN;
}
/**
 * \brief Main message receiver handler
 *
 * Routes incoming messages to appropriate handler based on message type.
 * Falls back to default handler for unknown message types.
 *
 * \param ctx Application context
 * \param sockfd Socket file descriptor of sender
 * \param msg Raw message string received
 */
void handle_message_receive(AppContext *ctx, int sockfd, const char *msg)
{
    if (!msg || strlen(msg) == 0)
        return;

    char msg_copy[MAX_MESSAGE_LENGTH * 4];
    strncpy(msg_copy, msg, sizeof(msg_copy));
    msg_copy[sizeof(msg_copy) - 1] = '\0';

    char command[MAX_MESSAGE_LENGTH];
    sscanf(msg_copy, "%s", command);
    MessageType type = get_message_type(command);

    for (int i = 0; i < sizeof(strategies) / sizeof(strategies[0]); ++i)
    {
        if (strategies[i].type == type)
        {
            char words[MAX_ARGUMENTS][MAX_MESSAGE_LENGTH] = {0};
            int count = split_string(msg, words, strategies[i].max_words);
            strategies[i].handler(ctx, sockfd, words, count);
            return;
        }
    }

    // Default handler
    log_msg(LOG_INFO, "Default message received: %s", msg);
}

/**
 * \brief Gets string prefix for message type
 *
 * \param type MessageType enum value
 * \return Corresponding protocol string prefix
 */
static const char *get_message_type_prefix(MessageType type)
{
    switch (type)
    {
    case MSG_SEND:
        return "send";
    case MSG_TERMINATED:
        return "terminate";
    case MSG_CONNECTED:
        return "connect";
    case MSG_EXIT:
        return "exit";
    default:
        return "unknown";
    }
}
/**
 * \brief Builds formatted message buffer for sending
 *
 * Constructs protocol message according to specified type and parameters.
 * Handles buffer overflow protection.
 *
 * \param buffer Output buffer for constructed message
 * \param buf_size Size of output buffer
 * \param type Message type
 * \param sender_ip Sender's IP address
 * \param sender_port Sender's port number
 * \param body Message body content
 * \return true if message built successfully, false otherwise
 */
static bool build_message_buffer(char *buffer, size_t buf_size, MessageType type,
                                 const char *sender_ip, uint16_t sender_port, const char *body)
{
    if (!buffer || !sender_ip || !body)
        return false;

    const char *type_str = get_message_type_prefix(type);

    int written = 0;

    switch (type)
    {
    case MSG_CONNECTED:
    case MSG_TERMINATED:
        // Format: <type> <sender_ip> <sender_port>
        written = snprintf(buffer, buf_size, "%s %s %u", type_str, sender_ip, sender_port);
        break;
    case MSG_EXIT:
        // Format: <type> <sender_ip> <sender_port>
        written = snprintf(buffer, buf_size, "%s %s %u", type_str, sender_ip, sender_port);
        break;

    case MSG_SEND:
        // Format: send <sender_ip> <sender_port> <message>
        written = snprintf(buffer, buf_size, "%s %s %u %s", type_str, sender_ip, sender_port, body);
        break;

    default:
        written = snprintf(buffer, buf_size, "unknown");
        break;
    }

    if (written < 0 || (size_t)written >= buf_size)
    {
        fprintf(stderr, "⚠️ Message too long. Truncating.\n");
        buffer[buf_size - 1] = '\0';
        return false;
    }

    return true;
}
/**
 * \brief Handles message sending to peer
 *
 * Formats and sends message to specified peer connection.
 * Logs errors for invalid arguments or send failures.
 *
 * \param ctx Application context
 * \param receiver Target peer connection
 * \param type Message type to send
 * \param message Message content
 * \return true if message sent successfully, false otherwise
 */
bool handle_message_send(AppContext *ctx, PeerConnection *receiver,
                         MessageType type, const char *message)
{
    if (!ctx || !receiver || !message || !ctx->peer_manager)
    {
        log_msg(LOG_ERR, "Invalid arguments in send_message_to_peer()");
        return false;
    }

    char msg_buffer[256];
    const char *sender_ip = ctx->local_info.ip_address;
    uint16_t sender_port = ctx->local_info.port;

    if (!build_message_buffer(msg_buffer, sizeof(msg_buffer),
                              type, sender_ip, sender_port, message))
    {
        log_msg(LOG_ERR, "Failed to build message buffer");
        return false;
    }

    size_t len = strnlen(msg_buffer, 256);
    if (len > 100)
        len = 100; // Limit actual send length

    ssize_t sent = send(receiver->socket_fd, msg_buffer, len, 0);
    if (sent != (ssize_t)len)
    {
        log_msg(LOG_ERR, "Failed to send full message (sent %zd/%zu bytes)", sent, len);
        return false;
    }

    return true;
}