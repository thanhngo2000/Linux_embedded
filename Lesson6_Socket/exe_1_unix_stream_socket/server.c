
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/unix_stream_socket"

int main()
{
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char buffer[100];

    // file socket delete if exit
    unlink(SOCKET_PATH);

    // create socket
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // set address socket socket
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // listen client
    listen(server_fd, 5);
    printf("Server listening\n");

    while (1)
    {
        // accept connection
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0)
        {
            perror("accept failed");
            continue; // listen new connection
        }

        while (1)
        {
            ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
            if (bytes_read > 0)
            {
                buffer[bytes_read] = '\0'; //
                printf("Received: %s\n", buffer);
            }
            else
            {
                perror("read failed");
                break;
            }
        }

        // close client socket
        close(client_fd);
    }

    // close server socket
    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}