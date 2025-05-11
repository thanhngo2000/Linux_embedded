#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include <signal.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>

//==========================//
//        CONSTANTS         //
//==========================//
#define MAX_PEERS 100
#define MAX_IP_ADDRESS_LENGTH 64
#define MAX_MESSAGE_LENGTH 100
#define MAX_INPUT_BUFFER 256
#define MAX_ARGUMENTS 32
#define MAX_CMD 32
#define MAX_THREADS 128

//==========================//
//   STRUCT: PeerConnection //
//==========================//
typedef struct
{
    int id;
    int socket_fd;
    char ip_address[MAX_IP_ADDRESS_LENGTH];
    uint16_t port;
    bool active;
} PeerConnection;

//===============================//
//  STRUCT: PeerConnectionManager //
//===============================//
typedef struct
{
    PeerConnection peers[MAX_PEERS];
    int connection_count;
    pthread_mutex_t connection_mutex;
} PeerConnectionManager;

//==============================//
//  STRUCT: ParsedUserCommand   //
//==============================//
typedef struct
{
    char input[MAX_INPUT_BUFFER];                    // input string
    char command[MAX_CMD];                           // Command name: connect, send,...
    char arguments[MAX_ARGUMENTS][MAX_INPUT_BUFFER]; // Argument in command
    int argument_count;
} ParsedUserCommand;

//===========================//
//    STRUCT: LocalHostInfo //
//===========================//
typedef struct
{
    char ip_address[MAX_IP_ADDRESS_LENGTH];
    uint16_t port;
    int socket_fd;
} LocalHostInfo;
//===========================//
//    STRUCT: ThreadManager //
//===========================//
typedef struct
{
    pthread_t threads[MAX_THREADS];
    int count;
    pthread_mutex_t lock;
} ThreadManager;

//=============================//
//     STRUCT: AppContext      //
//=============================//
typedef struct
{
    PeerConnectionManager *peer_manager; // connection manager
    LocalHostInfo local_info;            // host local info
    pthread_mutex_t console_mutex;       //
    ThreadManager thread_manager;        // thread manager
} AppContext;

#endif // SHARED_DATA_H