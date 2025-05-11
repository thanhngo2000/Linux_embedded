
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define PORT 8080

int main()
{
    int sock;
    struct sockaddr_in addr;
    char buffer[100];

    // create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket creation failed");
        return EXIT_FAILURE;
    }

    // set struct sockaddr_in
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(PORT);

    // connect to server
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect failed");
        close(sock);
        return EXIT_FAILURE;
    }

    // send data
    printf("Connected to server. Type your messages below:\n");

    // sen message
    while (1)
    {
        // enter message
        printf("> ");
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        {
            perror("fgets failed");
            break;
        }

        // skip empty string
        if (strcmp(buffer, "\n") == 0)
        {
            continue;
        }

        // send message to server
        if (send(sock, buffer, strlen(buffer), 0) < 0)
        {
            perror("send failed");
            break; //
        }
    }
    close(sock);
    return EXIT_SUCCESS;
}
