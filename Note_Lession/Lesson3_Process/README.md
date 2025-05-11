# Process

## Phân Biệt Giữa Chương Trình (Program) và Tiến Trình (Process)

### Định Nghĩa

- **Chương Trình (Program)**: Tập hợp các câu lệnh lưu trong file, không thực thi cho đến khi chạy.
- **Tiến Trình (Process)**: Phiên bản đang thực thi của một chương trình, bao gồm mã, dữ liệu và trạng thái.

### Tình Trạng

- **Chương Trình**: Không có trạng thái; không sử dụng tài nguyên cho đến khi thực thi.
- **Tiến Trình**: Có trạng thái (đang chạy, chờ, tạm dừng), sử dụng tài nguyên hệ thống.

### Tài Nguyên

- **Chương Trình**: Không sử dụng tài nguyên cho đến khi chạy.
- **Tiến Trình**: Sử dụng CPU, bộ nhớ và tài nguyên khác.

### Thực Thi

- **Chương Trình**: Cần biên dịch và chạy để thực thi.
- **Tiến Trình**: Đang thực thi, có thể tương tác với người dùng và các tiến trình khác.

### Số Lượng

- **Chương Trình**: Nhiều chương trình giống nhau có thể tồn tại mà không tương tác.
- **Tiến Trình**: Nhiều phiên bản của một chương trình có thể chạy đồng thời.

## Phân biệt inode của file và PID của process

PID là mã định danh của file, duy nhất trong 1 process.

Còn PID của process là mã định danh của process là duy nhất trong toàn bộ hệ thống
Hàm lấy PID của process cha và process con

- getpid(): trả về PID của tiến trình con
- getppid(): trả về PID của tiến trình cha

## Command-line Arguments

### Giới thiệu

Mỗi một chương trình đều bắt đầu khởi chạy từ hàm `main`.

- Khi chạy chương trình, các tham số môi trường (command-line arguments) sẽ được truyền qua hai đối số trong hàm `main`.

### Cú pháp

```c
int main(int argc, char *argv[])
```

- argc: Số lượng tham số truyền vào hàm main.
- argv[]: Mảng con trỏ tới tất cả các số đối số truyền cho chương trình đó.

## Memory Layout

### Giới Thiệu

Bố cục bộ nhớ của một tiến trình là cách mà bộ nhớ được phân chia và tổ chức cho từng tiến trình trong hệ thống.

Hiểu rõ về bố cục này giúp lập trình viên tối ưu hóa việc sử dụng tài nguyên và tăng hiệu suất ứng dụng.

### Các Thành Phần của Bố Cục Bộ Nhớ

**Text Segment**

- Chứa mã lệnh của chương trình (code).
- Thường là vùng nhớ chỉ đọc để bảo vệ mã khỏi bị thay đổi.

**Initialized Data Segment**

- Chứa các biến toàn cục và tĩnh đã được khởi tạo.
- Ví dụ: `int a = 5;`

**Uninitialized Data Segment (BSS)**

- Chứa các biến toàn cục và tĩnh chưa được khởi tạo.
- Tự động được khởi tạo về 0 khi tiến trình bắt đầu.

**Heap**

- Vùng bộ nhớ dùng cho cấp phát động.
- Các biến được cấp phát tại thời điểm chạy (runtime) thông qua các hàm như `malloc()` trong C/C++.
- Phát triển từ dưới lên.

**Stack**

- Dùng để lưu trữ các biến cục bộ và thông tin hàm (như địa chỉ trở về).
- Phát triển từ trên xuống.
- Quản lý theo kiểu LIFO (Last In, First Out).

### Kết Luận

Hiểu rõ về bố cục bộ nhớ của một tiến trình là rất quan trọng trong lập trình hệ thống và tối ưu hóa hiệu suất ứng dụng.

Việc quản lý bộ nhớ hiệu quả giúp tránh các lỗi như tràn bộ nhớ (memory overflow) và rò rỉ bộ nhớ (memory leak).

## Thao Tác Với Tiến Trình Trong Embedded Linux

#### 1. Tạo Tiến Trình

- **fork()**: Tạo một tiến trình con từ tiến trình cha.

  ```c
  pid_t pid = fork();
  ```

#### 2. Thực Thi Tiến Trình

exec(): Thay thế tiến trình hiện tại bằng một chương trình mới.

```c
execl("/path/to/program", "program_name", (char *)NULL);
```

#### 3. Chờ Tiến Trình

wait(): Chờ cho một tiến trình con kết thúc.

```c
pid_t pid = wait(&status);
```

#### 4. Kết Thúc Tiến Trình

exit(): Kết thúc một tiến trình.

```c
exit(0);
```

#### 5. Quản Lý Tiến Trình

kill(): Gửi tín hiệu đến một tiến trình để thực hiện các hành động như dừng hoặc kết thúc.

```c
kill(pid, SIGKILL);
```

pthread để quản lý các tiến trình và luồng.

```c
#include <pthread.h>

void *myThreadFunc(void *arg) {
    // Code cho luồng
}

pthread_t thread;
pthread_create(&thread, NULL, myThreadFunc, NULL);
pthread_join(thread, NULL);
```

#### Kết Luận

Quản lý tiến trình trong Embedded Linux là một kỹ năng quan trọng giúp lập trình viên phát triển các ứng dụng hiệu quả và ổn định.

Việc sử dụng các lệnh và thư viện phù hợp sẽ giúp tối ưu hóa hiệu suất của hệ thống.

## Orphan Process and Zombie Proces

### Orphan Process

#### Định Nghĩa

- **Orphan Process** là một tiến trình sinh ra mà không có tiến trình cha đang hoạt động. Điều này xảy ra khi tiến trình cha kết thúc trước tiến trình con.

### Cách Xử Lý

- Khi một tiến trình cha kết thúc, hệ thống sẽ tự động chuyển giao quyền quản lý tiến trình con cho tiến trình `init` (PID 1).
- `init` sẽ trở thành cha nuôi của tiến trình con và quản lý nó cho đến khi nó hoàn thành.

#### Ví dụ

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();

    if (pid > 0) {
        // Tiến trình cha kết thúc
        exit(0);
    } else if (pid == 0) {
        // Tiến trình con
        sleep(10);  // Giả lập công việc
        printf("Orphan process\n");
    }

    return 0;
}
```

### Zombie Process

#### Định Nghĩa

- Zombie Process là một tiến trình đã hoàn thành nhưng vẫn còn tồn tại trong bảng tiến trình của hệ thống.
- Tiến trình này không còn thực thi, nhưng vẫn giữ lại thông tin như PID và trạng thái cho tiến trình cha.

#### Nguyên Nhân

- Zombie process xảy ra khi tiến trình cha không gọi hàm wait() để lấy thông tin kết thúc của tiến trình con, dẫn đến việc tiến trình con vẫn còn được ghi nhận trong hệ thống.

#### Cách Xử Lý

- Để xử lý zombie process, tiến trình cha cần gọi wait() hoặc waitpid() để thu thập trạng thái của tiến trình con.

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid > 0) {
        // Tiến trình cha
        sleep(5);  // Giả lập công việc
        printf("Parent process finished\n");
    } else if (pid == 0) {
        // Tiến trình con
        exit(0);  // Kết thúc tiến trình con
    }

    // Không gọi wait() sẽ tạo ra zombie process
    return 0;
}
```
