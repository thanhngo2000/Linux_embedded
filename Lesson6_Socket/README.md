# Excercise

**Different between stream socket and datagram socket**

- in stream socket, after client connect to server, server have to check and accept connection from client -> data safe
- in datagram socket, after client connect to server, it don't have step check connection -> data unsafe

**Different between UNIX socket and INTERNET socket**

- in UNIX communicate between server and client in same computer and through a socket file
- in internet communicate between server and client can be in the same or different computer and with IP vs port connection

## Exercise 1

### Client UNIX stream

- create socket UNIX stream

```bash
sock = socket(AF_UNIX, SOCK_STREAM, 0);
```

- server connect

```bash
connect(sock, (struct sockaddr *)&addr, sizeof(addr));
```

- Get data from user

```bash
fgets(buffer, sizeof(buffer), stdin);
```

- send message to server socket

```bash
send(sock, buffer, strlen(buffer), 0);
```

### Server UNIX stream

- create socket

```bash
socket(AF_UNIX, SOCK_STREAM, 0);
```

- bind socket with address

```bash
bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
```

- wait for client connect

```bash
listen(server_fd, 5);
```

- accept connect to from client

```bash
client_fd = accept(server_fd, NULL, NULL);
```

- close socket

```bash
close(server_fd);
```

- unlink socket

```bash
unlink(SOCKET_PATH);
```

### explain code

#### server

- unlink socket path if altready exit
- create socket
- set address and socket path for socket
- bind socket with address
- wait for client connect (listen)
- if accept client connection
- receive message to client
- close socket client and server
- unlink socket path

#### client

- create socket
- set address and socket path for socket
- connect to server
- if connect success, wait for user input and send to server
- after finish close socket

**Build**

```copy
make
```

**Run server**

```copy
./server
```

**Run client**

```copy
./client
```

**Clean**

```copy
make clean
```

## Exercise 2

### Client UNIX datagram

- create socket

```bash
socket(AF_UNIX, SOCK_DGRAM, 0);
```

- set struct sockadd_in

```bash
memset(&addr, 0, sizeof(struct sockaddr_un));
addr.sun_family = AF_UNIX;
strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
```

- send message

```bash
sendto(sock, message, strlen(message), 0, (struct sockaddr *)&addr, sizeof(addr))
```

### Server UNIX datagram

- create socket

```bash
socket(AF_UNIX, SOCK_DGRAM, 0);
```

- read message

```bash
bytes_read = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &len);
```

### Explain code

#### server

- unlink socket path if altready exit
- create socket
- set address and socket path for socket
- bind socket with address
- wait for receiving message to client
- close socket server
- unlink socket path

#### client

- create socket
- set address and socket path for socket
- connect to server
- wait for user input and send to server
- after finish close socket
  **Build**

```copy
make
```

**Run server**

```copy
./server
```

**Run client**

```copy
./client
```

**Clean**

```copy
make clean
```

## Exercise 3

### Client ipv4 stream

- create socket

```bash
socket(AF_INET, SOCK_STREAM, 0);
```

- set ip and port

```bash
memset(&addr, 0, sizeof(struct sockaddr_in));
addr.sin_family = AF_INET;
addr.sin_addr.s_addr = inet_addr("127.0.0.1");
addr.sin_port = htons(PORT);
```

### Server ipv4 stream

- connect server with Ip and port

```bash
bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
```

- read message

```bash
read(client_fd, buffer, sizeof(buffer) - 1);
```

### Explain code

#### server

- create socket
- set IP and port
- bind socket with ip and port
- wait for client connect (listen)
- if accept client connection
- receive message to client
- close socket client and server

#### client

- create socket
- set ip and port
- connect to server
- if connect success, wait for user input and send to server
- after finish close socket
  **Build**

```copy
make
```

**Run server**

```copy
./server
```

**Run client**

```copy
./client
```

**Clean**

```copy
make clean
```

## Exercise 4

### Client ipv4 datagram

- create socket

```bash
socket(AF_INET, SOCK_DGRAM, 0);
```

- send message

```bash
sendto(sock, message, strlen(message), 0, (struct sockaddr *)&addr, sizeof(addr));
```

### Server ipv4 datagram

- create socket

```bash
socket(AF_INET, SOCK_DGRAM, 0);
```

- receive message

```bash
recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &len);
```

### Explain code

#### server

- create socket
- set ip and port
- bind socket with ip and port
- wait for receiving message to client
- close socket server

#### client

- create socket
- set ip and port
- connect to server
- wait for user input and send to server
- after finish close socket

**Build**

```copy
make
```

**Run server**

```copy
./server
```

**Run client**

```copy
./client
```

**Clean**

```copy
make clean
```
