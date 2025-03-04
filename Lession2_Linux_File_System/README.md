# Exceicise

**How to build**

```copy
make
```

**How to clean**

```copy
make clean
```

```copy
./app
```

## Exercise 1

The output will be printed at the next position in the data file without overwriting the previous data because:

- When opening a file with the flag O_APPEND, the pointer is moved to the end of the file.
- With the write command, the writing will begin at the end of the file, even if the lseek command has been used to move the pointer within the file.
- With the read command, the pointer can still be moved back to the beginning of the file to read the data.

## Exercise 3

0. example_grogram: file_name executive
1. filename: filename
2. num-bytes: num bytes r/w
3. [r/w]: r -> read from filename
   w -> write to filename
4. "Hello": content when write

- Example Read:

```copy
./app thanh 100 r
```

- Example Write:

```copy
./app thanh 100 w hello
```

## Exercise 4

- **case S_IFBLK**: If the file is a block device, print "block device".
- **case S_IFCHR**: If the file is a character device, print "character device".

- **case S_IFDIR**: If the file is a directory, print "directory".

- **case S_IFIFO**: If the file is a FIFO or pipe, print "FIFO/pipe".

- **case S_IFLNK**: If the file is a symlink, print "symlink".

- **case S_IFREG**: If the file is a regular file, print "regular file".

- **case S_IFSOCK**: If the file is a socket, print "socket".

Example

```copy
./app <filename>
```
