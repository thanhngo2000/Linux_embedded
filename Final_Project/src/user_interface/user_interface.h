#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H
#include "../../include/shared_data.h"
#include "../socket/socket.h"
#include "../logger/logger.h"
#include "../storage/storage.h"
#include "../connection/connection.h"
#include "../security/security.h"

// typedef struct Command Command;
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
void handle_command(const char *command_input);
#endif