# Signal

## 1. Khái niệm

- Signal là một software interrupt cơ chế dùng để để xử lý các sự kiện bất đồng bộ
- Signal Handler là một hàm được chỉ định để xử lý một loại signal cụ thể mà một process nhận được. Khi một signal đến, kernel sẽ tạm dừng process hiện tại và gọi hàm handler tương ứng để xử lý signal đó. Sau khi handler thực hiện xong, process có thể tiếp tục thực hiện từ nơi nó đã dừng lại.
- code ví dụ

```copy
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void signal_handler(int signum) {
    printf("Received signal %d\n", signum);
    exit(0); // Thoát chương trình
}

int main() {
    // Đăng ký signal handler cho SIGINT
    signal(SIGINT, signal_handler);

    printf("Process ID: %d\n", getpid());
    printf("Press Ctrl+C to send SIGINT.\n");

    // Vòng lặp vô hạn
    while (1) {
        sleep(1); // Giả lập công việc
    }

    return 0;
}

```

## 2. Một số flag signal

**SIGINT (Interrupt Signal)**

- Mô Tả: Gửi khi người dùng nhấn Ctrl+C trong terminal. Thường được sử dụng để yêu cầu process dừng lại.
- Ví Dụ: Khi bạn chạy một chương trình trong terminal và nhấn Ctrl+C, chương trình sẽ nhận được signal SIGINT.
  **SIGTERM (Termination Signal)**
- Mô Tả: Gửi để yêu cầu một process dừng lại. Đây là signal mặc định khi sử dụng lệnh kill.
- Ví Dụ: gửi signal này đến một process bằng cách sử dụng lệnh:

```copy
kill <pid>
```

**SIGKILL (Kill Signal)**

- Mô Tả: Gửi để yêu cầu một process dừng lại ngay lập tức. Không thể bị chặn hoặc xử lý bởi process.
- Ví Dụ

```copy
kill -9 <pid>
```

**SIGSTOP (Stop Signal)**

- Mô Tả: Gửi để tạm dừng một process. Không thể bị chặn hoặc xử lý.
- Ví Dụ: Sử dụng lệnh:

```copy
kill -STOP <pid>
```

để tạm dừng một process. Để tiếp tục, bạn có thể sử dụng:

```copy
kill -CONT <pid>
```

**SIGQUIT (Quit Signal)**

- Mô Tả: Gửi khi người dùng nhấn Ctrl+\ trong terminal. Thường được sử dụng để yêu cầu process dừng và tạo ra core dump.
- Ví Dụ: Khi bạn chạy một chương trình trong terminal và nhấn Ctrl+\, chương trình sẽ nhận được signal SIGQUIT.

## 3. Các lệnh blocking và unblocking signal

- sigemptyset: clear toàn bộ signal mask
- sigfillset: thêm toàn bộ các singal và singal mask
- sigaddset: chỉ định thêm singal vào singal mask
- sigdelset: xoá singal ra khỏi singal mask
- sigismember: kiểm ra xem signal có phải là member của singal mask không

## 4. sigaction

- là một hàm trong Linux dùng để thiết lập hành vi của chương trình khi nhận tín hiệu.
- Nó cho phép chỉ định một hàm xử lý tín hiệu (signal handler) cho một tín hiệu cụ thể.

```copy
    void (*sa_handler)(int);  // Hàm xử lý tín hiệu
    sigset_t sa_mask;          // Tập hợp tín hiệu bị chặn trong khi xử lý
    int sa_flags;              // Các cờ điều khiển hành vi
    void (*sa_sigaction)(int, siginfo_t *, void *); // Hàm xử lý tín hiệu (mới)
```

## 5. Select và Poll

- được sử dụng để theo dõi nhiều file descriptors (FD) và kiểm tra xem có FD nào sẵn sàng cho việc đọc, ghi hay có lỗi không
- select: Theo dõi nhiều file descriptors và chờ cho đến khi một hoặc nhiều trong số chúng sẵn sàng cho hành động.

```copy
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
```

- Poll: Giống như select, nhưng không có giới hạn về số lượng file descriptors và hoạt động với cấu trúc pollfd.

```copy
int poll(struct pollfd *fds, nfds_t nfds, int timeout);
```
