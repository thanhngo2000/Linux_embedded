# Exceicise

## Exercise 1

- register signal SIGINT handler

```bash
signal(SIGINT, signal_handler)
```

- if skip SIGINT, when you press ctrl + C, the program will stop
  **Program explaination**
- when press ctrl + C in keybroad, signal_handler wil be called and increase count variable
- when you pressed ctrl + C 3 times, it will call exit and stop running
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

- register signal SIGALRM with signal_handler

```bash
signal(SIGALRM, signal_handler)
```

- set up timer for alarm

```bash
alarm(1);
```

- if not call alarm in process, count will not be reseted and program will stop in while loop to wait for a signal
  **Code explain**
- register SIGALRm with signal handler
- set time for alram in 1s, after 1s will call signal_handler
- call alarm(1) in while loop for reset timer each seconds
- after 10 times, program call exit and stop running
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

- register SIGUSR1 with signal_handler

```bash
signal(SIGUSR1, signal_handler)
```

- send signal SIGUSR1 by PID

```bash
kill(pid, SIGUSR1);
```

- kill is the simple way to send signal between parent and child process instead of other way like pipe, socket,...
  **Code explain**
- register SIGUSR1 with signal_handler
- each 2s, process parent will send SIGUSR1 to child process by PID
- child process use pause() to stop and wait for signal from parent
- Each tiem receive signal, count will increase
- after 5 times, child process call exit and stop running program

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

## Exercise 4

- sigaction empty set

```bash
sigemptyset(&sa.sa_mask);
```

- register SIGTSTP for sigaction

```bash
sigaction(SIGTSTP, &sa, NULL)
```

- check SIGTSTP is member of sigaction list

```bash
sigismember(&sa.sa_mask, SIGTSTP)
```

- if you don't handler SIGTSTP, when press ctrl + Z in keyboard, program will stop and you can continue working in terminal
  **Code explaination**
- Create sigaction set
- register SIGTSTP for sigactions set
- check SIGTSTP is member of list sigaction
- when press ctrl + Z in keyboard, it will run signal_handler and print SIGTSTP ignored
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

## Exercise 5

- add SIGINT to sigaction

```bash
sigaction(SIGINT, &sa, NULL)
```

- check SIGINT is member of sigaction

```bash
sigismember(&sa.sa_mask, SIGINT)
```

- use select to wait for data from stdin or signa

```bash
result = select(max_fd + 1, &readfds, NULL, NULL, NULL);
```

**Code explaination**

- register SIGINT for sigaction and signal_handler
- if receive SIGINT from user, ignore it and pront SIGINT received
- if receive SIGTERM from user, termanate program

```copy
make
```

```copy
./app
```

**Send SIGINT**: ctrl + C
**send SIGTERM**

```copy
kill <pid>
```

**Clean**

```copy
make clean
```

## Exercise 6
