#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include <netinet/in.h>
#include <unistd.h>

#include "../include/project_config.h"
#include "../include/client_server.h"
#include "../utils/utils.h"
#include "../client/client.h"

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/
#define NUM_CMD_HELP 1
#define NUM_CMD_MYIP 1
#define NUM_CMD_MYPORT 1
#define NUM_CMD_CONNECT 3
#define NUM_CMD_LIST 1
#define NUM_CMD_TERMINATE 2
#define NUM_CMD_SEND 3
#define NUM_CMD_EXIT 1
/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/

/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/

/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
void print_menu();
void CommandHandler_handleCommand(PeerInfo *peer, const char *command);
void cmd_connect_handler(const char *command);
void cmd_terminate_handler(const char *command);
void cmd_send_handler(const char *command);
void cmd_exit_handler(const char *command);

#endif // USER_INTERFACE_H