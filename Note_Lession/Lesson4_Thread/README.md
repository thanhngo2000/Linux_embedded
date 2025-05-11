# Thread

## Khái Niệm Về Thread

- **Thread** là đơn vị nhỏ nhất của tiến trình, có thể thực thi độc lập. Nhiều thread có thể chạy trong cùng một tiến trình, chia sẻ bộ nhớ và tài nguyên của tiến trình đó.
- Thread nhẹ hơn tiến trình, cho phép tiết kiệm tài nguyên và giảm thời gian chuyển đổi ngữ cảnh.

## Các Thao Tác Cơ Bản Với Thread

### 1. Tạo Thread

Sử dụng thư viện `pthread.h` để tạo và quản lý thread.

```c
#include <pthread.h>

void *myThreadFunction(void *arg) {
    // Code cho luồng
    return NULL;
}

int main() {
    pthread_t thread;
    pthread_create(&thread, NULL, myThreadFunction, NULL);
    pthread_join(thread, NULL);  // Chờ thread kết thúc
    return 0;
}
```

### 2. Đồng Bộ Hóa Thread

- Để tránh xung đột dữ liệu khi nhiều thread truy cập vào tài nguyên chung, cần sử dụng mutex (khóa).

```c
#include <pthread.h>

pthread_mutex_t myMutex;

void *myThreadFunction(void *arg) {
    pthread_mutex_lock(&myMutex);
    // Truy cập tài nguyên chung
    pthread_mutex_unlock(&myMutex);
    return NULL;
}
```

### 3. Sử Dụng Điều Kiện (Condition Variables)

Condition variables cho phép một thread chờ cho một điều kiện nhất định xảy ra trước khi tiếp tục thực thi.

1. Khởi tạo: Sử dụng pthread_cond_init().
2. Chờ: Sử dụng pthread_cond_wait().
3. Báo thức: Sử dụng pthread_cond_signal() hoặc pthread_cond_broadcast().
4. Hủy: Sử dụng pthread_cond_destroy().

```copy
#include <pthread.h>

pthread_cond_t myCondition;
pthread_mutex_t myMutex;

void *producer(void *arg) {
    pthread_mutex_lock(&myMutex);
    // Sản xuất dữ liệu
    pthread_cond_signal(&myCondition);  // Thông báo cho consumer
    pthread_mutex_unlock(&myMutex);
    return NULL;
}

void *consumer(void *arg) {
    pthread_mutex_lock(&myMutex);
    pthread_cond_wait(&myCondition, &myMutex);  // Chờ dữ liệu
    // Xử lý dữ liệu
    pthread_mutex_unlock(&myMutex);
    return NULL;
}
```

### 4. Quản lý thread termination

- Thread kết thúc khi gọi pthread_exit()
- Thread bị hủy bỏ khi gọi hàm pthread_exit()
- Thu giá trị kết thúc từ thread khác pthread_join()
- Detach thread để các thread không lấy được giá trị kết thúc thông qua hàm join: pthread_join(thread_ID)

### So Sánh Process và Thread trong Linux

#### 1. Khái Niệm

- **Process**:

  - Một process (tiến trình) là một thể hiện của một chương trình đang chạy. Mỗi process có không gian bộ nhớ riêng biệt, tài nguyên riêng, và được quản lý độc lập bởi hệ điều hành.
  - Ví dụ: Khi bạn chạy một chương trình như trình duyệt web, hệ điều hành tạo ra một process riêng cho nó.

- **Thread**:
  - Một thread (luồng) là một đơn vị thực thi nhỏ nhất trong một process. Một process có thể chứa nhiều thread, và các thread này chia sẻ cùng không gian bộ nhớ và tài nguyên của process.
  - Ví dụ: Trong một ứng dụng đa luồng, các thread có thể chạy song song để thực hiện các tác vụ khác nhau.

---

#### 2. Đặc Điểm

| Đặc Điểm              | Process                                                                               | Thread                                                                                    |
| --------------------- | ------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------- |
| **Không gian bộ nhớ** | Mỗi process có không gian bộ nhớ riêng biệt.                                          | Các thread chia sẻ cùng không gian bộ nhớ của process.                                    |
| **Tài nguyên**        | Mỗi process có tài nguyên riêng (file descriptor, biến môi trường, v.v.).             | Các thread chia sẻ tài nguyên của process.                                                |
| **Tạo và hủy**        | Tốn nhiều thời gian và tài nguyên hơn.                                                | Nhanh hơn và ít tốn tài nguyên hơn.                                                       |
| **Giao tiếp**         | Giao tiếp giữa các process phức tạp hơn (IPC - Inter-Process Communication).          | Giao tiếp giữa các thread dễ dàng hơn do chia sẻ bộ nhớ.                                  |
| **Độc lập**           | Các process độc lập với nhau, lỗi trong một process không ảnh hưởng đến process khác. | Các thread phụ thuộc lẫn nhau, lỗi trong một thread có thể ảnh hưởng đến toàn bộ process. |
| **Hiệu suất**         | Tốn nhiều tài nguyên hệ thống hơn.                                                    | Hiệu quả hơn về mặt tài nguyên và thời gian chuyển đổi ngữ cảnh.                          |

---

#### 3. Kết Luận

- **Process** phù hợp cho các tác vụ độc lập, yêu cầu bảo mật và không chia sẻ tài nguyên.
- **Thread** phù hợp cho các tác vụ cần chia sẻ dữ liệu và tài nguyên, đồng thời yêu cầu hiệu suất cao.

## Khóa pthread_rwlock_t

### 1. Định nghĩa

- Được sử dụng để định nghĩa một khóa đọc-ghi (read-write lock).
- Nó cho phép quản lý đồng bộ giữa nhiều luồng khi truy cập vào một tài nguyên chung, như một biến hoặc một cấu trúc dữ liệu.
- Khóa Đọc (Read Lock): Cho phép nhiều luồng đọc cùng lúc, miễn là không có luồng nào đang ghi.
- Khóa Ghi (Write Lock): Cho phép chỉ một luồng ghi tại một thời điểm. Khi một luồng đang giữ khóa ghi, không có luồng nào khác có thể đọc hoặc ghi, đảm bảo rằng dữ liệu không bị thay đổi trong khi nó đang được ghi.

### 2. Cách sử dụng

Để sử dụng pthread_rwlock_t, bạn cần thực hiện các bước sau:
Khai báo biến:

```copy
pthread_rwlock_t rwlock;
```

Khởi tạo khóa:

```copy
pthread_rwlock_init(&rwlock, NULL);
```

Sử dụng khóa:

```copy
pthread_rwlock_rdlock(&rwlock) //for read
pthread_rwlock_wrlock(&rwlock) //for write
```

Giải phóng khóa:

```copy
pthread_rwlock_destroy:
pthread_rwlock_unlock(&rwlock);
pthread_rwlock_destroy(&rwlock);
```
