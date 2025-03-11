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
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("socket creation failed");
        return EXIT_FAILURE;
    }

    // set struct sockaddr_in
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    // bind socket
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

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
    return EXIT_SUCCESS;
}
