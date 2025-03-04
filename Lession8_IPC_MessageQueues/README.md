# Exceicise

## Exercise 1

- create amessage queues

```bash
mqd_t mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);
```

- receive message

```bash
mq_receive(mq, buffer, MAX_SIZE, NULL)
```

- send message

```bash
mq_receive(mq, buffer, MAX_SIZE, NULL)
```

- close message queue

```bash
mq_close(mq);
```

- deleate message queue

```bash
mq_unlink(QUEUE_NAME);
```

**Code explain**

- create messagequeue
- in parent process, send message to message queue
- in child process, read message in message queue

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

- create messgae queue send message from parent to child

```bash
mq_request = mq_open(QUEUE_REQUEST_NAME, O_CREAT | O_RDWR, 0644, &attr);
```

- create message queue send number character in message from chile to parent

```bash
mq_response = mq_open(QUEUE_RESPONSE_NAME, O_CREAT | O_RDWR, 0644, &attr);
```

- close messgae queue

```bash
mq_close(mq_request);
mq_close(mq_response);
```

- delete message queue

```bash
mq_unlink(QUEUE_REQUEST_NAME);
mq_unlink(QUEUE_RESPONSE_NAME);
```

**Code explain**

- parent send message to child by mq_request message queue
- child process receive message from parent by mq_request message queue and send number of chacracter by message_response message queue to parent
- parent receive number character by message_response message queue and print

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

\*Code explain\*\*

- create message queue
- parent send messgae to child 1
- child 1 receive message and convent message to uppercase. Then send to child 2
- child 2 receive message form child 1 and print

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
