#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/unix_datagram_socket"

int main()
{
    int sock;
    struct sockaddr_un addr;
    char buffer[100];

    unlink(SOCKET_PATH); // unlink to socket path first

    // create socket
    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("socket creation failed");
        return EXIT_FAILURE;
    }

    // config address
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // assign socket
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind failed");
        close(sock);
        return EXIT_FAILURE;
    }

    // Data receive
    while (1)
    {
        int len = sizeof(addr);
        int bytes_read = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &len);
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

    close(sock);
    unlink(SOCKET_PATH);
    return EXIT_SUCCESS;
}