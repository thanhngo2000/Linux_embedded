# Exceicise

## Exercise 1

Print PID child in child process and PID child in parent process
Build program

```copy
make
```

Run program

```copy
./app
```

Clean program

```copy
clean
```

## Exercise 2

- After call function execlp() or execvp(), it will replace all process child code with new program.
- After that, child process will not continue
- Parent process still work normally

Run ls

```copy
./app 1
```

Run date

```copy
./app 2
```

## Exercise 3

- Use flag signal SIGUSR1 for process child

```bash
signal(SIGUSR1, handle_sigusr1)
```

- parent process wait 2s, after that send signal to continue child process

```bash
kill(pid, SIGUSR1);
```

Run program

```copy
./app
```

## Exercise 4

**Define exit status codes**

- #define EXIT_GENERAL_ERROR 1 // General error
- #define EXIT_FILE_NOT_FOUND 2 // File not found
- #define EXIT_COMMAND_CANNOT_EXEC 126 // Command invoked cannot execute
- #define EXIT_COMMAND_NOT_FOUND 127 // Command not found
- #define EXIT_INTERRUPTED 130 // Process terminated by Ctrl+C
- #define EXIT_KILLED 137 // Process killed (received SIGKILL)

WIFEXITED() and WEXITSTATUS(): get exit code of child process

- Check status code is valid

```bash
WIFEXITED(status)
```

- Git exit code

```bash
int exit_code = WEXITSTATUS(status)
```

Run code

```copy
./app
```

## Exercise 4

**Create zombie process**

- zombie process create when process child is end before parent process start
- in the code, process child is start and running about 2 second, then end but parent process is stuck in while(1). So child process becomes zombie process
- check zombie process

```copy
ps aux | grep Z
```

**create orphan process**

- orphan process create when process parent end while process child still running.
- in the code, process parent is end after 2s starting but the child process still in while (1). so child process become orphan process
- check orphan process

```copy
ps aux | grep orphan
```

**Build zombie process**

```copy
gcc -o app_zombie process_zombie.c
```

**Build orphan process**

```copy
gcc -o app_orphan process_orphan.c
```

**Build both**

```copy
make all
```

**Clean**

```copy
make clean
```
