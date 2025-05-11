#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/unix_stream_socket"

int main()
{
    int sock;
    struct sockaddr_un addr;
    char buffer[100];

    // create socket
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket creation failed");
        return EXIT_FAILURE;
    }

    // sẻ address socket
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // connect server
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect failed");
        close(sock);
        return EXIT_FAILURE;
    }

    printf("Connected to server. Type your messages below:\n");

    // send message
    while (1)
    {
        printf("> ");
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        {
            perror("fgets failed");
            break; // error
        }

        // skip empty string
        if (strcmp(buffer, "\n") == 0)
        {
            continue;
        }

        // send message
        if (send(sock, buffer, strlen(buffer), 0) < 0)
        {
            perror("send failed");
            break;
        }
    }

    // close socket
    close(sock);
    return EXIT_SUCCESS;
}