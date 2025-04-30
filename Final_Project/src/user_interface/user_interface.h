#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "../../include/shared_data.h"
#include "../socket/socket.h"
#include "../logger/logger.h"
#include "../storage/storage.h"
#include "../connection/connection.h"
#include "../security/security.h"
/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/
// command pattern
typedef struct Command
{
    void (*execute)(struct Command *self, const char *command_input);
} Command;

// structure command
typedef struct
{
    const char *name;         // command name
    int expected_param_count; // num param
    Command *(*create)(void); // create command
} CommandSpec;
/******************************************************************************/
/*                            FUNCTIONS PROTOTYPES                             */
/******************************************************************************/
void handle_command(const char *command_input);
#endif