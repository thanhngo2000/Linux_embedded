
#ifndef UTILS_H
#define UTILS_H
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include <ifaddrs.h>

#include "../include/project_config.h"
#include "../include/client_server.h"
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
void handle_error(const char *message);
int check_user_input(const char *input, const int desired_count);
void split_string(const char *input, char words[MAX_WORDS][MAX_WORD_LENGTH], int desired_count);
char *get_ip();
bool is_port_in_list(const int port_connect);
bool is_valid_ip(const char *ip);
bool is_valid_port(const int port);
bool is_id_in_list(const int id);
bool is_same_port(const int port);

#endif // UTILS_H
