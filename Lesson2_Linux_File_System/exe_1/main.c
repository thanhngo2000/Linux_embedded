
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 256

int main(void)
{
	int fd;
	int num_read, num_write;
	char buf_write_hello[12] = "Hello";
	char buf_write_thanh[12] = "Thanh";
	char buf_read[BUFFER_SIZE];

	// open file
	fd = open("hello.txt", O_RDWR | O_CREAT | O_APPEND, 0667); // write to EOF file
	if (fd == -1)
	{
		perror("Cannot read file\n");
		close(fd);
		return EXIT_FAILURE;
	}

	// write hello to file
	num_write = write(fd, buf_write_hello, strlen(buf_write_hello));
	if (num_write == -1)
	{
		perror("write file failed\n");
		close(fd);
		return EXIT_FAILURE;
	}

	// make file pointer to top file
	lseek(fd, 0, SEEK_SET);

	// //write thanh to file
	num_write = write(fd, buf_write_thanh, strlen(buf_write_thanh));
	if (num_write == -1)
	{
		printf("write file failed\n");
		close(fd);
		return EXIT_FAILURE;
	}

	// make file pointer to top file
	lseek(fd, 0, SEEK_SET);

	// read from file again
	num_read = read(fd, buf_read, BUFFER_SIZE - 1);
	if (num_read == -1)
	{
		perror("read file failed\n");
		close(fd);
		return -1;
	}
	// add null to buff
	buf_read[num_read] = '\0';
	// printf data
	printf("File content: %s\n", buf_read);

	// close file
	close(fd);

	return EXIT_SUCCESS;
}