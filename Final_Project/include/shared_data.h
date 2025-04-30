#ifndef SHARED_DATA_H
#define SHARED_DATA_H
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "ctype.h"

#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sqlite3.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/sysinfo.h>
#include <signal.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/
#define MAX_CONNECTIONS 100
#define BUFFER_SIZE 256
#define RING_BUFFER_SIZE 1024
#define MAX_WORDS 10
#define MAX_WORD_LENGTH 100

#define OPENSSL_API_COMPAT 0x10100000L

#define LOG_FIFO_NAME "logFifo"
#define LOG_FILE_NAME "gateway.log"
#define DB_FILE_NAME "sensor_data.db"

#define SQL_CONNECTED "CONNECTED"
#define SQL_DISCONNECTED "DISCONNECTED"
#define SQL_RETRY_LIMIT 3
#define SQL_RETRY_DELAY_SEC 5

#define TEMPERATURE_HISTORY_SIZE 5

#define MAX_CONNECTIONS_PER_IP 5
#define MAX_UNIQUE_IPS 256

/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/
// ─── SSL ENCRYPTION security ──────────────────────────────────────────────────────────────
//
typedef enum
{
    SECURE_PLAIN,
    SECURE_SSL_CLIENT,
    SECURE_SSL_SERVER
} SecureMode;
typedef struct
{
    int (*send)(void *self, const char *data, size_t len);
    int (*recv)(void *self, char *buffer, size_t len);
    void (*close)(void *self);
} SecureCommunicationInterface;

// Abstract Connection class (Strategy Pattern)
typedef struct
{
    SecureCommunicationInterface interface;
    void *impl;
} SecureCommunication;
// SSL Connection
typedef struct
{
    SSL *ssl;
    SSL_CTX *ctx;
    int fd;
} SSLConnection;

typedef struct
{
    int fd;
} PlainConnection;

// ─── DOMAIN MODEL ──────────────────────────────────────────────────────────────
//
typedef enum
{
    CONN_STATUS_CONNECTED,
    CONN_STATUS_DISCONNECTED,
    CONN_STATUS_TIMEOUT
} ConnectionStatus;
typedef struct
{
    int timestamp;     // Epoch time
    int sensor_id;     // Unique ID per sensor
    float temperature; // Measured temperature
    bool is_valid;     // Data validity
} SensorData;

typedef struct
{
    float average;
    int count;
} RunningAverage;

typedef struct
{
    int socket_fd;
    char ip_address[INET_ADDRSTRLEN];
    int port;
    int sensor_id;

    bool is_active;
    time_t connected_time;
    time_t last_active_time;

    ConnectionStatus status;

    SecureCommunication *secure_comm;
} SensorConnection;

//
// ─── CONNECTION MANAGER ────────────────────────────────────────────────────────
//

typedef struct ConnectionNode
{
    SensorConnection connection;
    SensorData latest_data;
    struct ConnectionNode *next;
} ConnectionNode;

typedef struct
{
    ConnectionNode *head;
    int active_count;
    int running_port; // save port running
    pthread_mutex_t mutex;

    void (*add)(struct ConnectionNode **, SensorConnection, SensorData);
    void (*remove)(struct ConnectionNode **, int sensor_id);
    struct ConnectionNode *(*find)(struct ConnectionNode *, int sensor_id);
    void (*update)(struct ConnectionNode *, int sensor_id, SensorData);
    void (*display)(ConnectionNode *);
} ConnectionManager;

//
// ─── LOGGING ───────────────────────────────────────────────────────────────────
//
typedef enum
{
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_DEBUG
} LogLevel;
typedef struct
{
    int log_id;
    LogLevel level;
    time_t timestamp;
    char source[64];   // name module: Connection / Data / Storage
    char message[256]; // log message
} LogEvent;

typedef struct
{
    int fifo_fd;
    pthread_mutex_t mutex;
    int log_count;

} FifoLogger;
typedef struct LogManager
{

    void (*log)(struct LogManager *self, LogLevel level, const char *source, const char *message);
    void (*destroy)(struct LogManager *self);
    void *impl;
} LogManager;

// Factory
//
// ─── SQL STORAGE MANAGER ───────────────────────────────────────────────────────
//

typedef struct
{
    char status[20]; // CONNECTED / DISCONNECTED
    int retry_count;
    time_t last_retry_time;
    sqlite3 *db_handle;
} SQLConnectionInfo;

typedef struct
{
    SQLConnectionInfo sql_info;
    pthread_mutex_t mutex;
    int total_messages_received;
    ConnectionNode *pending_data_head; // linked lists contain waiting data
} StorageManager;

// Struct: DataManager - manage data
//--------------------------------------------------
typedef struct
{
    float history[TEMPERATURE_HISTORY_SIZE];
    int count;
    int index;
} TemperatureHistory;

typedef struct
{
    float hot_threshold;
    float cold_threshold;
    pthread_mutex_t mutex;
    TemperatureHistory *sensor_histories;
} DataManager;
//
// ─── SECURITY MANAGER ───────────────────────────────────────────────────────
//

typedef struct
{
    char ip[INET_ADDRSTRLEN];
    int count;
} IpEntry;

typedef struct
{
    IpEntry entries[MAX_UNIQUE_IPS];
    int size;
    pthread_mutex_t mutex;
} IpLimiterManager;

//
// ─── CENTRAL SYSTEM MANAGER ────────────────────────────────────────────────────
//

typedef struct
{
    ConnectionManager connection_manager;
    LogManager log_manager;
    StorageManager storage_manager;
    DataManager data_manager;

    IpLimiterManager ip_limiter_manager; // security

    // SSL_CTX *ssl_context;
    SSL_CTX *ssl_server_context;
    SSL_CTX *ssl_client_context;
} SystemManager;

extern SystemManager system_manager;
extern volatile sig_atomic_t stop_requested;

#endif