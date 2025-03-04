# Exceicise

## Exercise 1

- create pipe

```bash
pipe(pipefd)
```

- read from pipe

```bash
read(pipefd[HEAD_READ], buffer, sizeof(buffer))
```

- write from pipe

```bash
write(pipefd[HEAD_WRITE], message, strlen(message) + 1)
```

- close pipe read

```bash
close(pipefd[HEAD_READ]);
```

- close pipe write

```bash
close(pipefd[HEAD_WRITE])
```

- note: when use pipe read, need to close pipe write and when use pipe wite need to close pipe read
  **Code explain**
- create pipe read head with pipefd[0] and pipe write head with pipefd[1]
- parent process will write to write pipe head, before write need to close pipe read
- child process will read from read pipe head, before read need to close pipe write

**Build and run**

```copy
make
```

```copy
./app
```

**Clean**

```copy
make clean
```

## Exercise 2

- create pipe for parent and child connect

```bash
pipe(pipefd_parent_2_child)
```

- creae pipe for child to child connect

```bash
pipe(pipefd_child_2_child)
```

**Code explain**

- in parent process, parent write content to child with pipefd_parent_2_child
- in child 1 process, child 1 add connent and transfer content received from parent to child 2 with pipe pipefd_child_2_child
- in child 2 process, child 2 will read content receive and print

**Build and run**

```copy
make
```

```copy
./app
```

**Clean**

```copy
make clean
```

## Exercise 3

**Code explain**

- in parent process, parent will write messge to write head pipe
- in child process, child will read message received from parent, then count number character and print
  **Build and run**

```copy
make
```

```copy
./app
```

**Clean**

```copy
make clean
```
