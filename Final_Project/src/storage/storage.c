#include "storage.h"

static bool initialize_sql_table(sqlite3 *db)
{
    const char *create_table_sql = "CREATE TABLE IF NOT EXISTS sensor_data "
                                   "(id INTEGER PRIMARY KEY, timestamp INTEGER, "
                                   "sensor_id INTEGER, temperature REAL)";
    return sqlite3_exec(db, create_table_sql, NULL, NULL, NULL) == SQLITE_OK;
}
static void handle_sql_connection_success(SQLConnectionInfo *sql)
{
    strcpy(sql->status, SQL_CONNECTED);
    sql->retry_count = 0;
    system_manager.log_manager.log(&system_manager.log_manager,
                                   LOG_INFO, "Storage",
                                   "Connected to SQL database");
}
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

static ConnectionNode *create_data_node(SensorData data)
{
    ConnectionNode *new_node = malloc(sizeof(ConnectionNode));
    if (new_node == NULL)
        return NULL;

    new_node->latest_data = data;
    new_node->next = NULL;
    return new_node;
}
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

    printf("Storage manager cleanup completed.\n");
}
// handle pending data
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
void *storage_manager(void *arg)
{
    while (1)
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
