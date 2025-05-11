#ifndef MESSAGE_H
#define MESSAGE_H
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "../../include/shared_data.h"
#include "../connection/connection.h"
#include "../logger/logger.h"

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/
typedef enum
{
    MSG_UNKNOWN,
    MSG_CONNECTED,
    MSG_TERMINATED,
    MSG_SEND,
    MSG_EXIT
} MessageType;
typedef struct
{
    MessageType type;
    int max_words; // số từ cần split tương ứng với loại message
    void (*handler)(AppContext *, int, char words[][MAX_MESSAGE_LENGTH], int count);
} MessageHandlerStrategy;
/******************************************************************************/
/*                            FUNCTIONS PROTOTYPES                             */
/******************************************************************************/
void handle_message_receive(AppContext *ctx, int sockfd, const char *msg);
bool handle_message_send(AppContext *ctx, PeerConnection *receiver,
                         MessageType type, const char *message);
#endif