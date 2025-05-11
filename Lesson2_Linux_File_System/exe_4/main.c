
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

void print_info_file(const char *file_name);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Argument: %s <tên_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    print_info_file(argv[1]);
    return EXIT_SUCCESS;
}

void print_info_file(const char *file_name)
{
    struct stat file_stat;
    if (stat(file_name, &file_stat) < 0)
    {
        perror("cannot get file information");
        return;
    }
    // get file name
    printf("File name: %s\n", file_name);

    // get data type
    printf("File type: ");
    switch (file_stat.st_mode & S_IFMT) /// S_IFMT defined <sys/stat.h> to get file-type.
                                        //& to sepate file-type.
    {
    case S_IFBLK:
        printf("block device\n");
        break;
    case S_IFCHR:
        printf("character device\n");
        break;
    case S_IFDIR:
        printf("directory\n");
        break;
    case S_IFIFO:
        printf("FIFO/pipe\n");
        break;
    case S_IFREG:
        printf("regular file\n");
        break;
    default:
        printf("unknown\n");
        break;
    }

    // print time last modified
    printf("Creation time: %s\n", ctime(&file_stat.st_ctime));
    printf("Access time: %s\n", ctime(&file_stat.st_atime));
    printf("Last modification: %s\n", ctime(&file_stat.st_mtime));

    // note: if results date and time wrong need to check time with cmd: date and update timezone with cmd: sudo timedatectl set-timezone Asia/Ho_Chi_Minh

    // print file size
    printf("File size: %ld bytes\n", file_stat.st_size);
}