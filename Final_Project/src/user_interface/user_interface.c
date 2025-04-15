#include "user_interface.h"

/*------------------variable------------------*/
typedef struct
{
    Command base;
} ConnectCommand;

typedef struct
{
    Command base;
} TerminateCommand;

typedef struct
{
    Command base;
} LogCommand;
typedef struct
{
    Command base;
} ClearLogCommand;
typedef struct
{
    Command base;
} StatusCommand;
typedef struct
{
    Command base;
} StatsCommand;
typedef struct
{
    Command base;
} ReaddbCommand;
/*----------------------------------------------*/

/*Function prototype*/
Command *create_command_handler(const char *command_input);

/*------------------------------------------------*/

static void split_string(const char *input, char words[MAX_WORDS][MAX_WORD_LENGTH], int *word_count)
{
    char *token;
    char *input_copy = strdup(input);
    if (!input_copy)
    {
        perror("strdup failed");
        return;
    }

    *word_count = 0;
    token = strtok(input_copy, " ");
    while (token != NULL && *word_count < MAX_WORDS)
    {
        strncpy(words[*word_count], token, MAX_WORD_LENGTH);
        words[*word_count][MAX_WORD_LENGTH - 1] = '\0';
        (*word_count)++;
        token = strtok(NULL, " ");
    }
    free(input_copy);
}

/*--------------------------------------------------*/
void handle_command(const char *command_input)
{
    Command *command = create_command_handler(command_input);
    if (command == NULL)
    {
        printf("Error: Unknown command '%s'\n", command_input);
        return;
    }

    command->execute(command, command_input);
    free(command);
}
/*----------------command connect handler-------------------------------*/
static void execute_connect_command(Command *self, const char *command_args)
{
    printf("Processing connect command\n");

    char info_connect[MAX_WORDS][MAX_WORD_LENGTH];
    int param_count = 0;
    split_string(command_args, info_connect, &param_count);

    const int port_connect = atoi(info_connect[2]);

    char ip_connect[INET_ADDRSTRLEN];
    strcpy(ip_connect, info_connect[1]);
    ip_connect[INET_ADDRSTRLEN - 1] = '\0';

    const int running_port = system_manager.connection_manager.running_port;

    // check valid connection param
    if (!validate_connection_params(ip_connect, port_connect, running_port))
    {
        return;
    }

    printf("Connecting to %s:%d\n", ip_connect, port_connect);
    // run client thread connect to server
    start_client_connection(ip_connect, port_connect);
}
Command *create_connect_command(void)
{
    ConnectCommand *command = malloc(sizeof(ConnectCommand));
    if (command == NULL)
    {
        fprintf(stderr, "Memory allocation failed for connect command\n");
        return NULL;
    }
    command->base.execute = execute_connect_command;
    return (Command *)command;
}
/*------------------------------------------------------------*/

/*----------------command log handler-------------------------------*/
static void execute_log_command(Command *self, const char *command_args)
{
    printf("Read log \n");
    read_log_file(LOG_FILE_NAME);
}

Command *create_log_command(void)
{
    LogCommand *command = malloc(sizeof(LogCommand));
    if (!command)
    {
        fprintf(stderr, "Memory allocation failed for terminate command\n");
        return NULL;
    }
    command->base.execute = execute_log_command;
    return (Command *)command;
}
/*-------------------------------------------------------------*/

/*----------------command clearlog handler-------------------------------*/
static void execute_clear_log_command(Command *self, const char *command_args)
{

    if (clear_log_file(LOG_FILE_NAME) == 0)
    {
        printf("Clear log \n");
    }
    else
    {
        perror("Failed clear log \n");
    }
}

Command *create_clear_log_command(void)
{
    ClearLogCommand *command = malloc(sizeof(ClearLogCommand));
    if (!command)
    {
        fprintf(stderr, "Memory allocation failed for terminate command\n");
        return NULL;
    }
    command->base.execute = execute_clear_log_command;
    return (Command *)command;
}
/*-------------------------------------------------------------*/

/*----------------command terminate handler-------------------------------*/
static void execute_terminate_command(Command *self, const char *command_args)
{
    printf("Terminating server \n");
    char info_terminate[MAX_WORDS][MAX_WORD_LENGTH];
    int param_count = 0;
    split_string(command_args, info_terminate, &param_count);
    int sensorID_terminate = atoi(info_terminate[1]);
    printf("Sensor id need to terminate: %d\n", sensorID_terminate);

    system_manager.connection_manager.remove(&system_manager.connection_manager.head, sensorID_terminate);

    char msg[256];
    snprintf(msg, sizeof(msg), "A sensor node with %d has closed the connection", sensorID_terminate);
    system_manager.log_manager.log(&system_manager.log_manager, LOG_INFO, "Connection", msg);
}

Command *create_terminate_command(void)
{
    TerminateCommand *command = malloc(sizeof(TerminateCommand));
    if (!command)
    {
        fprintf(stderr, "Memory allocation failed for terminate command\n");
        return NULL;
    }
    command->base.execute = execute_terminate_command;
    return (Command *)command;
}
/*-------------------------------------------------------------*/

/*----------------command status handler-------------------------------*/
static void execute_status_command(Command *self, const char *command_args)
{
    printf("Review status info \n");
    display_system_status();
}

Command *create_status_command(void)
{
    StatusCommand *command = malloc(sizeof(StatusCommand));
    if (!command)
    {
        fprintf(stderr, "Memory allocation failed for status command\n");
        return NULL;
    }
    command->base.execute = execute_status_command;
    return (Command *)command;
}
/*-------------------------------------------------------------*/

/*----------------command stas handler-------------------------------*/
static void execute_stats_command(Command *self, const char *command_args)
{
    printf("Review stats connection \n");
    system_manager.connection_manager.display(system_manager.connection_manager.head);
}

Command *create_stats_command(void)
{
    StatsCommand *command = malloc(sizeof(StatsCommand));
    if (!command)
    {
        fprintf(stderr, "Memory allocation failed for stats command\n");
        return NULL;
    }
    command->base.execute = execute_stats_command;
    return (Command *)command;
}
/*-------------------------------------------------------------*/

/*----------------command readdb handler-------------------------------*/
static void execute_readdb_command(Command *self, const char *command_args)
{
    printf("Read database \n");
    storage_print_all_data();
}

Command *create_readdb_command(void)
{
    ReaddbCommand *command = malloc(sizeof(ReaddbCommand));
    if (!command)
    {
        fprintf(stderr, "Memory allocation failed for readdb command\n");
        return NULL;
    }
    command->base.execute = execute_readdb_command;
    return (Command *)command;
}
/*-------------------------------------------------------------*/

/*----------------command other handler-------------------------------*/
/*-------------------------------------------------------------*/

/*-------------------Handle command error------------------------------*/

static void to_lower_case(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        str[i] = tolower(str[i]);
    }
}

// list command support
static const CommandSpec valid_commands[] = {
    {"connect", 2, create_connect_command},     // connect <id> <port>
    {"terminate", 1, create_terminate_command}, // terminate <sensorID>
    {"log", 0, create_log_command},             // log
    {"clearlog", 0, create_clear_log_command},  // clearlog
    {"status", 0, create_status_command},       // status
    {"stats", 0, create_stats_command},         // stats
    {"readdb", 0, create_readdb_command}        // readdb
};
#define NUM_COMMANDS (sizeof(valid_commands) / sizeof(valid_commands[0]))
/*-----------------------------------*/
/*------------Validate connection parram---------------------*/
Command *create_command_handler(const char *command_input)
{
    char words[MAX_WORDS][MAX_WORD_LENGTH];
    int word_count = 0;

    // spilt string
    split_string(command_input, words, &word_count);

    if (word_count == 0)
    {
        printf("Error: Empty command\n");
        return NULL;
    }

    // change command to lowercase
    to_lower_case(words[0]);

    // find command in valid command list
    for (int i = 0; i < NUM_COMMANDS; i++)
    {
        if (strcmp(words[0], valid_commands[i].name) == 0)
        {
            // check num param
            if ((word_count - 1) != valid_commands[i].expected_param_count)
            {
                printf("Error: Command '%s' expects %d parameters, got %d\n",
                       valid_commands[i].name,
                       valid_commands[i].expected_param_count,
                       word_count - 1);
                return NULL;
            }
            // return command
            return valid_commands[i].create();
        }
    }

    return NULL;
}
/*-----------------------------------*/