
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define NUM_ARGUMENTS_WRITE 5
#define NUM_ARGUMENTS_READ 4

int main(int argc, char *argv[])
{
    // argc: number arguments (include program name)
    // argv: array containing arguments

    int fd;
    if ((argc == NUM_ARGUMENTS_WRITE) || (argc == NUM_ARGUMENTS_READ))
    {
        char file_name[256];
        snprintf(file_name, sizeof(file_name), "%s.txt", argv[1]);
        fd = open(file_name, O_RDWR | O_CREAT, 0667);
        if (fd == -1)
        {
            printf("Error opening file\n");
            close(fd);
            return EXIT_FAILURE;
        }

        // number bytes want to read/write
        int num_bytes = atoi(argv[2]);
        if (num_bytes == 0)
        {
            printf("num-bytes must be greater than 0");
            close(fd);
            return EXIT_FAILURE;
        }
        printf("num_bytes r/w: %d\n", num_bytes);

        // byte read/write
        char action[2];
        action[0] = argv[3][0];
        action[1] = '\0';
        if (strcmp(action, "w") == 0)
        {
            // write file
            printf("Write file\n");
            char buffer_write[num_bytes];
            strcpy(buffer_write, argv[4]);
            write(fd, buffer_write, num_bytes); // write to file
        }
        else if (strcmp(action, "r") == 0)
        {
            // read file
            printf("Read file\n");
            char buffer_read[num_bytes];
            lseek(fd, 0, SEEK_SET);           // put file pointer into top
            read(fd, buffer_read, num_bytes); // read from file
            printf("Data read: %s\n", buffer_read);
        }
        else
        {
            printf("action must be r or w\n");
            close(fd);
            return EXIT_FAILURE;
        }
    }

    else // wrong num arguments
    {
        printf("Wrong number arguments. Program need %d or %d arguments.\n", NUM_ARGUMENTS_READ, NUM_ARGUMENTS_WRITE);
        close(fd);
        return EXIT_FAILURE;
    }

    close(fd);
    return EXIT_SUCCESS;
}