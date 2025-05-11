#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080

int main()
{
    int sock;
    struct sockaddr_in addr;
    char message[100];

    // create socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    // set struct sockaddr_in
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(PORT);

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
    return 0;
}
