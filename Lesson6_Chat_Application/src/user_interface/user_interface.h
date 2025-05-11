#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "../../include/shared_data.h"
#include "../connection/connection.h"
#include "../utils/utils.h"
#include "../socket/socket.h"
#include "../message/message.h"
/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/

#define NUM_ARGS_CMD_HELP 0
#define NUM_ARGS_CMD_MYIP 0
#define NUM_ARGS_CMD_MYPORT 0
#define NUM_ARGS_CMD_CONNECT 2
#define NUM_ARGS_CMD_LIST 0
#define NUM_ARGS_CMD_TERMINATE 1
#define NUM_ARGS_CMD_SEND 2
#define NUM_ARGS_CMD_EXIT 0

typedef void (*CommandExecutor)(AppContext *, ParsedUserCommand *);
typedef struct
{
    const char *name;
    CommandExecutor execute;
} CommandEntry;
/******************************************************************************/
/*                            FUNCTIONS PROTOTYPES                             */
/******************************************************************************/
bool parse_user_input(const char *input, ParsedUserCommand *parsed_cmd);
CommandEntry *dispatch_command(const ParsedUserCommand *cmd);
void *user_command_loop(void *arg);
#endif // USER_INTERFACE_H