/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "storage.h"
/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
/**
 * \brief Initializes the sensor data table in the SQL database if it does not exist.
 *
 * \param db The SQLite database handle.
 *
 * \return Returns true if the table is created successfully or already exists, false otherwise.
 *
 * \note This function creates a table named `sensor_data` with columns `id`, `timestamp`, `sensor_id`, and `temperature`.
 */
static bool initialize_sql_table(sqlite3 *db)
{
    const char *create_table_sql = "CREATE TABLE IF NOT EXISTS sensor_data "
                                   "(id INTEGER PRIMARY KEY, timestamp INTEGER, "
                                   "sensor_id INTEGER, temperature REAL)";
    return sqlite3_exec(db, create_table_sql, NULL, NULL, NULL) == SQLITE_OK;
}
/**
 * \brief Handles a successful SQL connection by updating the connection status and logging the event.
 *
 * \param sql The SQL connection information structure.
 *
 * \note This function updates the connection status to "connected" and resets the retry count.
 *       It logs the connection success message.
 */
static void handle_sql_connection_success(SQLConnectionInfo *sql)
{
    strcpy(sql->status, SQL_CONNECTED);
    sql->retry_count = 0;
    system_manager.log_manager.log(&system_manager.log_manager,
                                   LOG_INFO, "Storage",
                                   "Connected to SQL database");
}
/**
 * \brief Handles a failed SQL connection by updating the connection status and retrying if the limit is not reached.
 *
 * \param sql The SQL connection information structure.
 *
 * \return Returns true if the retry limit is not reached and the connection should be retried, false otherwise.
 *
 * \note This function increments the retry count and logs an error message if the maximum retry limit is reached.
 *       It sleeps for a delay before retrying.
 */
static bool handle_sql_connection_failure(SQLConnectionInfo *sql)
{
    strcpy(sql->status, SQL_DISCONNECTED);
    sql->retry_count++;
    sql->last_retry_time = time(NULL);

    if (sql->retry_count >= SQL_RETRY_LIMIT)
    {
        system_manager.log_manager.log(&system_manager.log_manager,
                                       LOG_ERROR, "Storage",
                                       "Max retry attempts reached. Shutting down.");
        exit(EXIT_FAILURE);
    }
    sleep(SQL_RETRY_DELAY_SEC);
    return false;
}
/**
 * \brief Attempts to connect to the SQL database with retry logic.
 *
 * \return Returns true if the connection is successful, false if retries are exhausted.
 *
 * \note This function tries to open the database connection, creates the table, and handles any connection failures
 *       by retrying the connection process.
 */
static bool storage_connect_with_retry()
{
    SQLConnectionInfo *sql = &system_manager.storage_manager.sql_info;

    if (sqlite3_open(DB_FILE_NAME, &sql->db_handle) == SQLITE_OK)
    {
        if (initialize_sql_table(sql->db_handle))
        {
            handle_sql_connection_success(sql);
            return true;
        }
        sqlite3_close(sql->db_handle);
    }
    return handle_sql_connection_failure(sql);
}
/**
 * \brief Creates a new connection node to hold sensor data.
 *
 * \param data The sensor data to be stored in the new node.
 *
 * \return Returns a pointer to the newly created connection node, or NULL if memory allocation fails.
 *
 * \note This function allocates memory for a new connection node and stores the provided sensor data.
 */
static ConnectionNode *create_data_node(SensorData data)
{
    ConnectionNode *new_node = malloc(sizeof(ConnectionNode));
    if (new_node == NULL)
        return NULL;

    new_node->latest_data = data;
    new_node->next = NULL;
    return new_node;
}
/**
 * \brief Appends a new connection node to the pending data list.
 *
 * \param new_node The new connection node to be appended.
 *
 * \note This function traverses the pending data list and appends the new node at the end.
 */
static void append_to_pending_data(ConnectionNode *new_node)
{
    if (system_manager.storage_manager.pending_data_head == NULL)
    {
        system_manager.storage_manager.pending_data_head = new_node;
    }
    else
    {
        ConnectionNode *current = system_manager.storage_manager.pending_data_head;
        while (current->next != NULL)
            current = current->next;
        current->next = new_node;
    }
}
/**
 * \brief Adds sensor data to the pending data list.
 *
 * \param data The sensor data to be added.
 *
 * \note This function locks the mutex, creates a new data node, and appends it to the pending data list.
 *       If memory allocation fails, it logs an error message.
 */
void storage_add_data(SensorData data)
{
    pthread_mutex_lock(&system_manager.storage_manager.mutex);

    ConnectionNode *new_node = create_data_node(data);
    if (new_node != NULL)
    {
        append_to_pending_data(new_node);
    }
    else
    {
        system_manager.log_manager.log(&system_manager.log_manager,
                                       LOG_ERROR, "Storage",
                                       "Failed to allocate memory for new sensor data.");
    }

    pthread_mutex_unlock(&system_manager.storage_manager.mutex);
}
/**
 * \brief Initializes the storage manager, including setting initial values for SQL connection information and mutex.
 *
 * \note This function initializes the mutex and sets up the initial state of the SQL connection info, including
 *       the connection status and retry count.
 */
void init_storage_manager()
{
    pthread_mutex_init(&system_manager.storage_manager.mutex, NULL);
    strcpy(system_manager.storage_manager.sql_info.status, SQL_DISCONNECTED);
    system_manager.storage_manager.sql_info.retry_count = 0;
    system_manager.storage_manager.sql_info.last_retry_time = 0;
    system_manager.storage_manager.sql_info.db_handle = NULL;
    system_manager.storage_manager.pending_data_head = NULL;
    system_manager.storage_manager.total_messages_received = 0;
}
/**
 * \brief Cleans up resources used by the storage manager, including closing SQL connection and freeing memory.
 *
 * \note This function closes the database connection if open, frees all pending data nodes, and destroys the mutex.
 */
void cleanup_storage_manager()
{
    // 1. close SQL if open
    if (system_manager.storage_manager.sql_info.db_handle != NULL)
    {
        sqlite3_close(system_manager.storage_manager.sql_info.db_handle);
        system_manager.storage_manager.sql_info.db_handle = NULL;
        strcpy(system_manager.storage_manager.sql_info.status, SQL_DISCONNECTED);

        // write log
        system_manager.log_manager.log(&system_manager.log_manager,
                                       LOG_INFO, "Storage",
                                       "Closed SQL database connection");
    }

    // clean pending_data
    pthread_mutex_lock(&system_manager.storage_manager.mutex);
    ConnectionNode *current = system_manager.storage_manager.pending_data_head;
    while (current != NULL)
    {
        ConnectionNode *next = current->next;
        free(current);
        current = next;
    }
    system_manager.storage_manager.pending_data_head = NULL;
    pthread_mutex_unlock(&system_manager.storage_manager.mutex);

    // destroy mutex
    pthread_mutex_destroy(&system_manager.storage_manager.mutex);
}
/**
 * \brief Prepares an SQL statement for inserting sensor data into the database.
 *
 * \param db The SQLite database handle.
 * \param data The sensor data to be inserted.
 *
 * \return Returns the prepared statement for insertion, or NULL if preparation fails.
 *
 * \note This function binds the sensor data values to the prepared SQL statement for insertion.
 */
static sqlite3_stmt *prepare_insert_statement(sqlite3 *db, SensorData data)
{
    const char *insert_sql = "INSERT INTO sensor_data (timestamp, sensor_id, temperature) "
                             "VALUES (?, ?, ?)";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, data.timestamp);
        sqlite3_bind_int(stmt, 2, data.sensor_id);
        sqlite3_bind_double(stmt, 3, data.temperature);
        return stmt;
    }
    return NULL;
}
/**
 * \brief Removes a processed connection node from the pending data list.
 *
 * \param to_remove The connection node to be removed.
 * \param prev The previous node in the list.
 *
 * \note This function frees the memory used by the processed node and updates the pending data list.
 */
static void remove_processed_node(ConnectionNode *to_remove, ConnectionNode *prev)
{
    if (prev)
    {
        prev->next = to_remove->next;
    }
    else
    {
        system_manager.storage_manager.pending_data_head = to_remove->next;
    }
    free(to_remove);
    system_manager.storage_manager.total_messages_received++;
}
/**
 * \brief Processes all pending sensor data and inserts it into the database.
 *
 * \note This function iterates through the pending data list, prepares insert statements,
 *       executes them, and removes processed nodes from the list. Errors are logged.
 */
static void process_pending_data()
{
    pthread_mutex_lock(&system_manager.storage_manager.mutex);

    ConnectionNode *current = system_manager.storage_manager.pending_data_head;
    ConnectionNode *prev = NULL;
    SQLConnectionInfo *sql = &system_manager.storage_manager.sql_info;

    while (current != NULL)
    {
        sqlite3_stmt *stmt = prepare_insert_statement(sql->db_handle, current->latest_data);
        if (stmt != NULL)
        {
            if (sqlite3_step(stmt) == SQLITE_DONE)
            {
                ConnectionNode *next = current->next;
                remove_processed_node(current, prev);
                current = next;
            }
            else
            {
                const char *errmsg = sqlite3_errmsg(sql->db_handle);
                system_manager.log_manager.log(&system_manager.log_manager,
                                               LOG_ERROR, "Storage",
                                               errmsg);
                prev = current;
                current = current->next;
            }
            sqlite3_finalize(stmt);
        }
        else
        {
            prev = current;
            current = current->next;
        }
    }

    pthread_mutex_unlock(&system_manager.storage_manager.mutex);
}
/**
 * \brief Prints all sensor data from the database to the console.
 *
 * \note This function retrieves all sensor data from the database, formats the timestamp into a human-readable format,
 *       and prints the data in a table format. If the database is not connected, it logs a warning.
 */
void storage_print_all_data()
{
    SQLConnectionInfo *sql = &system_manager.storage_manager.sql_info;

    if (strcmp(sql->status, SQL_DISCONNECTED) == 0)
    {
        system_manager.log_manager.log(&system_manager.log_manager, LOG_WARNING,
                                       "Storage", "Cannot print DB. SQL not connected.");
        printf("Database not connected.\n");
        return;
    }

    const char *sql_query = "SELECT timestamp, sensor_id, temperature FROM sensor_data ORDER BY timestamp ASC";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(sql->db_handle, sql_query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        system_manager.log_manager.log(&system_manager.log_manager, LOG_ERROR,
                                       "Storage", "Failed to prepare SELECT statement.");
        return;
    }

    printf("\n%-20s %-15s %-10s\n", "Timestamp", "Sensor ID", "Temperature");
    printf("-------------------------------------------------------------\n");

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        int timestamp = sqlite3_column_int(stmt, 0);
        int sensor_id = sqlite3_column_int(stmt, 1);
        double temperature = sqlite3_column_double(stmt, 2);

        // Convert timestamp -> human-readable format
        time_t raw_time = (time_t)timestamp;
        struct tm *time_info = localtime(&raw_time);
        char time_str[20]; // Format: YYYY-MM-DD HH:MM:SS
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);

        printf("%-20s %-15d %-10.2f\n", time_str, sensor_id, temperature);
    }

    sqlite3_finalize(stmt);

    system_manager.log_manager.log(&system_manager.log_manager, LOG_INFO,
                                   "Storage", "All sensor data printed to console");
}
/**
 * \brief Main storage manager thread that handles SQL connections and processes pending data.
 *
 * \param arg A pointer to any arguments (unused).
 *
 * \note This function runs in a loop, attempting to connect to the SQL database if disconnected,
 *       and processing any pending data every second.
 */
void *storage_manager(void *arg)
{
    while (!stop_requested)
    {
        // connect to SQL if not connect
        if (strcmp(system_manager.storage_manager.sql_info.status, SQL_DISCONNECTED) == 0)
        {
            if (!storage_connect_with_retry())
                continue;
        }

        // handle pending data
        process_pending_data();

        sleep(1);
    }
    return NULL;
}
