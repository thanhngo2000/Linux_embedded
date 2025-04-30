# Sensor Gateway Project

## 📖 Overview

This project implements a robust and secure **Sensor Gateway System** for collecting, processing, and storing temperature data from multiple sensor nodes. It uses **multi-threading**, **inter-process communication**, **SQL storage**, and **secure socket communication** to create a scalable, high-performance gateway.

Sensor nodes send temperature data over TCP. The gateway processes this data, logs system events, and stores valid measurements in a SQL database. The system includes fault tolerance, logging, and security mechanisms.

---

## 🚀 How to Run

### ✅ Prerequisites

- GCC (C99/C11)
- SQLite3
- POSIX Threads (`pthreads`)
- Linux OS (for `epoll`, `fork`, FIFO, etc.)
- OpenSSL (for TLS/SSL support)

### 🛠️ Build the Project

```bash
make
```
### Clean the Project
```bash
clean
```

### 📦 Run project
```bash
./app <port>
```
```pass
123456
```

#### Example:
```bash
./app 4000
123456
```


### 🧠 System Architecture
```bash
├── cert.csr
├── cert.key
├── cert.pem
├── include
│   └── shared_data.h
├── Makefile
└── src
    ├── connection
    │   ├── connection.c
    │   └── connection.h
    ├── data
    │   ├── data.c
    │   └── data.h
    ├── logger
    │   ├── logger.c
    │   └── logger.h
    ├── main.c
    ├── security
    │   ├── security.c
    │   └── security.h
    ├── socket
    │   ├── socket.c
    │   └── socket.h
    ├── storage
    │   ├── storage.c
    │   └── storage.h
    ├── user_interface
    │   ├── user_interface.c
    │   └── user_interface.h
    └── utils
        ├── utils.c
        └── utils.h
```
## 💡 Features

### ✅ Sensor Connection Management

- Accepts multiple concurrent TCP connections 
- Uses `epoll` for scalable I/O multiplexing  
- Each sensor session includes:  
  - Unique ID  
  - IP/Port  
  - Connection uptime  
- Automatic disconnection after inactivity (timeout)  

- command connect sensor with server
```bash
connect <ip_server> <port_server>
```

- command check list active connection
```bash
stats
```
- command terminate sensor connection
```bash
terminate <sensor_id>
```
### ✅ Shared Data Buffer

- All 3 threads communicate through a single thread-safe shared buffer (`sbuffer`)  
- Protected by mutex and condition variables  

### ✅ Logging System

- Logs are sent via FIFO `logFifo` to a separate log process  
- Supports thread-safe FIFO writes  
- Logs are formatted as:  <event number> <timestamp> <message>
- Events include connection state, temperature status, SQL state, and errors  
- command check file log
```bash
log
```
- result
```bash
Read log
=== Log File (gateway.log) ===
=== Log file created ===
0|2025-04-30 20:20:23|Storage|Connected to SQL database
1|2025-04-30 20:20:41|Connection|A sensor node with 0 has opened a new connection
=== End of Log ===

```
- command clear file log
```bash
clearlog
```
## ✅ Temperature Monitoring Logic

- Calculates running average of temperature per sensor  
- Logs if temperature is **too hot** or **too cold**  

## ✅ Storage System

- Stores valid temperature data to SQLite  
- Handles SQL connection failures with retry logic (up to 3 attempts)  
- On repeated failure, the system shuts down gracefully  
- command read sql database
```bash
readdb
```
- result
```bash
Read database

Timestamp            Sensor ID       Temperature
-------------------------------------------------------------
2025-04-30 20:20:41  0               33.00
2025-04-30 20:20:44  0               36.00
2025-04-30 20:20:47  0               27.00
2025-04-30 20:20:50  0               15.00
2025-04-30 20:20:53  0               43.00
2025-04-30 20:20:56  0               35.00

```

## ✅ System Status Monitoring

- `status`: Show system metrics (CPU, RAM, active connections) 
- command show system status
```bash
status
``` 
- result
```bash
[System Status]
Active connections       : 1
 Total messages received : 3 (live buffer: 0)
 RAM: 3433 MB used / 4822 MB total
CPU cores: 4
``` 
- `stats`: Per-connection stats (uptime, traffic)  
- command how stats per-connection
```bash
stats
```
- result
```bash
Review stats connection

=== ACTIVE CONNECTIONS (1) ===
+------+-------------------+-------+------------+---------------------+---------------------+
|  ID  |      IP Address   | Port  |   Status   |    Connected Time   |   Last Active Time  |
+------+-------------------+-------+------------+---------------------+---------------------+
|    0 |    192.168.40.131 |  5000 |  CONNECTED | 2025-04-30 20:20:41 | 2025-04-30 20:20:44 |
+------+-------------------+-------+------------+---------------------+---------------------+

```

## 🔐 Security Features

- TLS/SSL encrypted communication using OpenSSL.
- code send message with ssl
- Authentication using token or username/password  
- Limits on max connections per IP (DoS protection)  

---

## 🔁 Fault Tolerance & Recovery

- Retry mechanism for SQL insertions  
- In-memory buffer holds data until DB connection is restored  
- State-aware crash recovery  
- Automatic reconnection logic  

## Check Valgrind memory leak result
```bash
==4972==
==4972== HEAP SUMMARY:
==4972==     in use at exit: 0 bytes in 0 blocks
==4972==   total heap usage: 23,255 allocs, 23,255 frees, 2,703,471 bytes allocated
==4972==
==4972== All heap blocks were freed -- no leaks are possible
==4972==
==4972== For lists of detected and suppressed errors, rerun with: -s
==4972== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

### 📌 Notes
- The logFifo is created automatically (handled in code)

- Logs are written to gateway.log line by line

- Use Ctrl+C for safe shutdown

### 📄 License
