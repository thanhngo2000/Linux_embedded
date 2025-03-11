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
    int server_fd, client_fd;
    struct sockaddr_in addr;
    char buffer[100];

    // create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // set struct sockaddr_in
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    // connect socket
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    listen(server_fd, 5);

    printf("Server is listening...\n");

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
        close(client_fd);
    }
    close(server_fd);
    return EXIT_SUCCESS;
}
