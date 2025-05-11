/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "../include/shared_data.h"
#include "user_interface/user_interface.h"
#include "connection/connection.h"
#include "utils/utils.h"
#include "socket/socket.h"
#include "logger/logger.h"
/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/
volatile sig_atomic_t keep_running = 1;
/******************************************************************************/
/*                            FUNCTIONS                                       */
/******************************************************************************/
/**
 * \brief Signal handler to gracefully terminate application
 *
 * Sets global flag to indicate application should stop running.
 * Typically triggered on SIGINT (Ctrl+C).
 *
 * \param sig Signal number received
 */
static void handle_signal(int sig)
{
     keep_running = false;
}
/**
 * \brief Initializes application context
 *
 * Allocates and initializes components in AppContext such as peer manager,
 * thread manager, console logger and local socket information.
 *
 * \param ctx Pointer to AppContext structure to initialize
 * \param port Port number to bind the local socket
 * \return true if initialization successful, false otherwise
 *
 * \note Must be called before using context-dependent functions
 */
static bool init_app_context(AppContext *ctx, uint16_t port)
{
     if (!ctx)
          return false;

     memset(ctx, 0, sizeof(AppContext));

     ctx->peer_manager = create_peer_manager();
     if (!ctx->peer_manager)
          return false;

     strncpy(ctx->local_info.ip_address, get_ip(), MAX_IP_ADDRESS_LENGTH);
     ctx->local_info.port = port;
     ctx->local_info.socket_fd = -1;

     thread_manager_init(&ctx->thread_manager);
     logger_init(&ctx->console_mutex);

     pthread_mutex_init(&ctx->console_mutex, NULL);
     return true;
}
/**
 * \brief Cleans up application context and resources
 *
 * Stops server socket, joins threads, frees peer manager and destroys console mutex.
 * Ensures graceful shutdown and resource deallocation.
 *
 * \param ctx Pointer to initialized AppContext
 *
 * \note Safe to call even if initialization failed partially.
 */
static void cleanup_app_context(AppContext *ctx)
{
     if (!ctx)
          return;

     keep_running = 0;

     log_msg(LOG_INFO, "Cleaning up context");
     socket_stop(ctx);
     thread_manager_join_all(&ctx->thread_manager); // Join all thread

     if (ctx->local_info.socket_fd > 0)
     {
          close(ctx->local_info.socket_fd);
     }

     if (ctx->peer_manager)
     {
          destroy_peer_manager(ctx->peer_manager);
     }

     pthread_mutex_destroy(&ctx->console_mutex);
}
/**
 * \brief Main user command loop
 *
 * Continuously prompts user for input, parses the command, and dispatches to appropriate handler.
 * Runs until global `keep_running` flag is set to false (e.g. via Ctrl+C).
 *
 * \param ctx Pointer to initialized AppContext
 */
static void run_command_loop(AppContext *ctx)
{
     char input[MAX_INPUT_BUFFER];

     while (keep_running)
     {
          printf("\nEnter command: ");
          fflush(stdout);

          if (!fgets(input, sizeof(input), stdin))
               break;
          input[strcspn(input, "\n")] = '\0';
          if (strlen(input) == 0)
               continue;

          ParsedUserCommand parsed_cmd = {0};
          if (!parse_user_input(input, &parsed_cmd))
          {
               log_msg(LOG_ERR, "❌ Failed to parse command");
               continue;
          }

          CommandEntry *cmd = dispatch_command(&parsed_cmd);
          if (cmd)
          {
               cmd->execute(ctx, &parsed_cmd);
          }
          else
          {
               log_msg(LOG_ERR, "❌ Unknown command: %s\n", parsed_cmd.command);
          }
     }
}
/*------main function-------*/
int main(int argc, char *argv[])
{
     if (argc != 2)
     {
          log_msg(LOG_ERR, "Usage: %s <port>\n", argv[0]);
          return EXIT_FAILURE;
     }

     uint16_t port = atoi(argv[1]);
     if (port <= 0 || port > 65535)
     {
          log_msg(LOG_ERR, "❌ Invalid port number. Please enter a port between 1 and 65535.");
          return EXIT_FAILURE;
     }

     signal(SIGINT, handle_signal);

     AppContext app_ctx;
     if (!init_app_context(&app_ctx, (uint16_t)atoi(argv[1])))
     {
          log_msg(LOG_ERR, "❌ Failed to initialize application context.");
          return EXIT_FAILURE;
     }

     server_start(&app_ctx);

     run_command_loop(&app_ctx);
     cleanup_app_context(&app_ctx);
     return EXIT_SUCCESS;
}