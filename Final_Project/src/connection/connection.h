#ifndef CONNECTION_H
#define CONNECTION_H

/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/

#include "../../include/shared_data.h"
#include "../socket/socket.h"
#include "../storage/storage.h"
#include "../utils/utils.h"
#include "../security/security.h"
/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/
#define CONNECTION_TIMEOUT_SECONDS 30
/******************************************************************************/
/*                            FUNCTIONS PROTOTYPE                             */
/******************************************************************************/
void *connection_manager(void *arg);
void init_connection_manager();
void cleanup_connection_manager();
void *check_timeout_connection_thread(void *arg);
void *update_sensor_data_thread(void *arg);
void display_system_status();

#endif
