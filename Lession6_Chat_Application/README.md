# Chat application

## Objective

The objective of this assignment is to familiarize yourself with socket programming by developing a simple chat application that enables message exchange among remote peers.

## Getting Started

For an introduction to socket programming, refer to [Beej's Socket Guide](http://beej.us/guide/bgnet).

## Implementation Details

### Programming Environment

You can use any programming language you are comfortable with. **Important Notes:**

1. Use TCP sockets for peer connections.
2. Handle multiple socket connections.

### Running Your Program

Build program

```cope
make
```

When you run your application, it will accept one command-line parameter indicating the port on which it will listen for incoming connections.

- For instance, to run your program named `chat` on port `4000`, use:

```copy
./chat 4000
```

Run your program on three different computers to perform message exchanges.

Functionality
Your program should function like a UNIX shell and provide the following commands:

**help**

- Displays available user interface options.

```copy
help
```

**myip**

- Shows the actual IP address of the computer (not 127.0.0.1)

```copy
myip
```

**myport**

- Displays the port on which the process is listening for incoming connections.

```copy
myport
```

**connect <destination> <port no>**

- Establishes a TCP connection to the specified destination IP and port.
- Invalid IPs should be rejected with an error message.
- Self-connections and duplicate connections should also be flagged.

```copy
connect <ip> <port>
```

list: Displays a numbered list of all active connections with the format:

**terminate <connection id>**

- Terminates the specified connection. Displays an error message if the connection does not exist.

```copy
terminate <id>
```

**send <connection id> <message>**

- Sends a message (up to 100 characters) to the specified connection.
- Displays a confirmation message on success.

```copy
send <id> <message>
```

**exit**

- Closes all connections and terminates the process, updating the connection lists of other peers.

```copy
exit
```

Clean program

```copy
make clean
```
