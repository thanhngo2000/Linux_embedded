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
    char message[100];

    // create socket
    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("socket create failed");
        return EXIT_FAILURE;
    }

    // set struct sockaddr_un
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // send data
    while (1)
    {
        printf("> ");
        // get message from user
        if (fgets(message, sizeof(message), stdin) == NULL)
        {
            perror("fgets failed");
            break;
        }

        // skip enter
        if (strcmp(message, "\n") == 0)
        {
            continue;
        }

        // send message
        if (sendto(sock, message, strlen(message), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            perror("sendto failed");
            close(sock);
            return EXIT_FAILURE;
        }
    }

    close(sock);
    return EXIT_SUCCESS;
}