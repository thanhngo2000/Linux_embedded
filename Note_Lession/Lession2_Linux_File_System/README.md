# Linux File System

## User space and Kernel space

Bộ nhớ trong hệ thống máy tính được chia thành hai khu vực chính: **User Space** và **Kernel Space**. Dưới đây là mô tả chi tiết về từng khu vực:

### 1. User Space

#### Khái Niệm

- **User Space** là phần bộ nhớ mà các tiến trình người dùng (user processes) hoạt động. Mỗi tiến trình chạy trong một không gian bộ nhớ riêng biệt, đảm bảo rằng chúng không thể truy cập trực tiếp vào bộ nhớ của các tiến trình khác hoặc vào kernel.

#### Đặc Điểm

- **Bảo Mật**: User space được bảo vệ để ngăn chặn các tiến trình người dùng truy cập vào dữ liệu của nhau, giúp tránh xung đột và bảo mật.
- **Hạn chế Truy Cập**: Các tiến trình trong user space không có quyền truy cập trực tiếp vào phần cứng hoặc vào kernel, mà phải sử dụng các hệ thống gọi (system calls) để tương tác với kernel.
- **Kích Thước**: Thường có kích thước lên tới 3GB trong hệ thống 32-bit (có thể thay đổi tùy vào cấu hình) và có thể lớn hơn trong hệ thống 64-bit.

#### Ứng Dụng

- Chạy các ứng dụng, trình duyệt, và các tiến trình của người dùng khác.

### 2. Kernel Space

#### Khái Niệm

- **Kernel Space** là phần bộ nhớ mà kernel (nhân hệ điều hành) hoạt động. Kernel có quyền truy cập vào tất cả các tài nguyên phần cứng và quản lý chúng.

#### Đặc Điểm

- **Quyền Truy Cập Cao**: Kernel có quyền truy cập đầy đủ vào mọi phần của bộ nhớ và phần cứng, cho phép quản lý tài nguyên hệ thống một cách hiệu quả.
- **Bảo Mật**: Kernel space cũng được bảo vệ để ngăn chặn các tiến trình người dùng làm hỏng hoặc thay đổi trạng thái của kernel.
- **Kích Thước**: Kích thước của kernel space thường khoảng 1GB trong hệ thống 32-bit, nhưng có thể lớn hơn trong hệ thống 64-bit, tùy thuộc vào cấu hình.

#### Ứng Dụng

- Quản lý tài nguyên phần cứng, điều khiển tiến trình, quản lý bộ nhớ, và xử lý các hệ thống gọi từ user space.

### Tóm Tắt

- **User Space**: Dành cho các tiến trình người dùng, có quyền truy cập hạn chế, và được bảo vệ để đảm bảo bảo mật.
- **Kernel Space**: Dành cho kernel, có quyền truy cập cao vào tất cả tài nguyên hệ thống, và quản lý các hoạt động cốt lõi của hệ điều hành.

## System Call trong Embedded Linux

### Giới thiệu

System call (gọi hệ thống) là một phương thức mà các chương trình ứng dụng sử dụng để tương tác với hệ thống kernel. Trong môi trường Embedded Linux, việc hiểu rõ các system call là rất quan trọng để phát triển và tối ưu hóa ứng dụng.

### Các System Call Chính

#### 1. `open()`

- **Chức năng**: Mở một file hoặc device.
- **Cú pháp**: `int open(const char *pathname, int flags);`
- **Trả về**: Descriptors file nếu thành công, -1 nếu thất bại.

#### 2. `read()`

- **Chức năng**: Đọc dữ liệu từ file descriptor.
- **Cú pháp**: `ssize_t read(int fd, void *buf, size_t count);`
- **Trả về**: Số byte đã đọc, 0 nếu EOF, -1 nếu thất bại.

#### 3. `write()`

- **Chức năng**: Ghi dữ liệu vào file descriptor.
- **Cú pháp**: `ssize_t write(int fd, const void *buf, size_t count);`
- **Trả về**: Số byte đã ghi, -1 nếu thất bại.

#### 4. `close()`

- **Chức năng**: Đóng file descriptor.
- **Cú pháp**: `int close(int fd);`
- **Trả về**: 0 nếu thành công, -1 nếu thất bại.

#### 5. `ioctl()`

- **Chức năng**: Thực hiện các thao tác điều khiển trên device.
- **Cú pháp**: `int ioctl(int fd, unsigned long request, ...);`
- **Trả về**: -1 nếu thất bại.

### Sử Dụng System Call

Để sử dụng các system call, bạn cần include các header file cần thiết trong mã nguồn C:

```c
#include <fcntl.h>   // cho open, close
#include <unistd.h>  // cho read, write
#include <sys/ioctl.h> // cho ioctl
```

## Sự Phân Biệt Giữa Bộ Nhớ (Memory) và Bộ Nhớ Lưu Trữ (Storage)

- **Bộ Nhớ (RAM)**:

  - Tạm thời, nhanh, mất dữ liệu khi tắt máy.
  - Dùng để chạy ứng dụng và xử lý dữ liệu tạm thời.

- **Bộ Nhớ Lưu Trữ (HDD/SSD)**:
  - Lâu dài, chậm hơn, lưu dữ liệu khi tắt máy.
  - Dùng để lưu trữ tệp và dữ liệu lâu dài.

### Bộ Nhớ Cache

Bộ Nhớ Cache là một loại bộ nhớ tạm thời, được sử dụng để tăng tốc độ truy cập dữ liệu và cải thiện hiệu suất của hệ thống. Dưới đây là một số điểm chính về bộ nhớ cache:

#### Khái Niệm

- Bộ Nhớ Cache là một vùng bộ nhớ nhỏ, nhanh hơn, nằm gần với CPU hơn so với các loại bộ nhớ khác như RAM hoặc bộ nhớ lưu trữ. Nó lưu trữ các dữ liệu và hướng dẫn mà CPU thường xuyên truy cập.

#### Đặc Điểm

1. **Tốc Độ Cao**: Bộ nhớ cache có tốc độ truy cập nhanh hơn nhiều so với RAM, điều này giúp CPU giảm thiểu thời gian chờ đợi khi cần dữ liệu.
2. **Dữ Liệu Tạm Thời**: Dữ liệu trong bộ nhớ cache thường là tạm thời và có thể thay đổi. Cache lưu trữ các dữ liệu mà CPU đã truy cập gần đây hoặc dự đoán sẽ truy cập trong tương lai.
3. **Cấu Trúc Nhiều Cấp**: Thường có nhiều cấp độ cache (L1, L2, L3).

#### Chức Năng

- **Giảm Thời Gian Truy Cập**: Khi CPU cần dữ liệu, nó sẽ kiểm tra cache trước. Nếu dữ liệu đã có trong cache (gọi là cache hit), thời gian truy cập sẽ giảm đáng kể.
- **Tăng Hiệu Suất**: Việc sử dụng bộ nhớ cache giúp cải thiện hiệu suất tổng thể của hệ thống, giảm thời gian truy cập dữ liệu từ RAM hoặc bộ nhớ lưu trữ.

## chmod

- là lệnh để thay đổi quyền truy cập cho tệp và thư mục trong Linux.
-     Quyền có thể được chỉ định bằng ký hiệu (u, g, o) hoặc bằng số (0-7).

### Quyền truy cập cho tệp và thư mục User, Group, và Other

**User (u)**

- Định nghĩa: Đây là người sở hữu tệp hoặc thư mục. Thông thường, khi một tệp được tạo ra, người dùng tạo ra nó trở thành người sở hữu.
- Quyền: Người sở hữu có thể có quyền đọc, ghi, và thực thi tệp hoặc thư mục.
  **Group (g)**
- Định nghĩa: Đây là nhóm người dùng mà tệp hoặc thư mục thuộc về. Một tệp có thể được gán cho một nhóm, và tất cả các thành viên trong nhóm đó sẽ có quyền truy cập theo nhóm.
- Quyền: Thành viên trong nhóm có thể có quyền đọc, ghi, và thực thi tệp hoặc thư mục, tùy thuộc vào quyền mà người sở hữu đã chỉ định.
  **Other (o)**
- Định nghĩa: Đây là tất cả những người dùng khác không phải là người sở hữu hoặc là thành viên trong nhóm.
- Quyền: Quyền truy cập cho "Other" thường được thiết lập để cho phép hoặc từ chối quyền truy cập cho tất cả những người dùng khác trên hệ thống.

### Quyền Truy Cập

- Read (r): Quyền đọc tệp hoặc thư mục.
- Write (w): Quyền ghi (thay đổi) tệp hoặc thư mục.
- Execute (x): Quyền thực thi tệp (đối với tệp chương trình) hoặc quyền truy cập vào thư mục.

#### Ví dụ Quyền Truy Cập

Khi bạn xem thông tin quyền của một tệp bằng lệnh ls -l, bạn sẽ thấy kết quả như sau:

```bash
-rw-r--r-- 1 user group 0 Feb  1 12:00 example.txt
```

- -rw- (User): Người sở hữu có quyền đọc và ghi.
- r-- (Group): Nhóm có quyền đọc.
- r-- (Other): Người khác có quyền đọc.

## Thư mục root

- là điểm khởi đầu của hệ thống file trong Linux, nơi chứa tất cả các tệp và thư mục khác.
- Quyền truy cập vào thư mục root thường bị giới hạn cho người dùng root để bảo vệ hệ thống.

### một số thư mục chính thường thấy trong thư mục root (/) của hệ thống Linux

**/bin**

- Chứa các tệp thực thi (binaries) của các lệnh cơ bản, như ls, cp, mv, rm. Những lệnh này có thể được sử dụng bởi tất cả người dùng.
  **/boot**
- Chứa các tệp cần thiết để khởi động hệ thống, bao gồm kernel và các tệp cấu hình khởi động.
  **/dev**
- Chứa các tệp thiết bị (device files) đại diện cho các thiết bị phần cứng trong hệ thống, như ổ cứng, thiết bị đầu vào/đầu ra.
  **/etc**
- Chứa các tệp cấu hình cho hệ thống và ứng dụng. Ví dụ: passwd (thông tin người dùng) và fstab (cấu hình hệ thống file).
  **/home**
- Chứa các thư mục người dùng. Mỗi người dùng thường có một thư mục con trong /home, chẳng hạn như /home/user1, /home/user2.
  **/lib**
- Chứa các thư viện chia sẻ cần thiết cho các chương trình trong /bin và /sbin. Thư mục này bao gồm các tệp thực thi hỗ trợ.

## Quản lý File trong Linux

- Quản lý tệp trong Linux là một phần quan trọng của hệ điều hành, cho phép người dùng và ứng dụng tương tác với tệp và thư mục.
- Dưới đây là khái niệm về các thành phần chính trong quản lý tệp: file descriptor table, open file table, và I-node table.

#### File Descriptor Table:

- Bảng lưu trữ thông tin về các tệp đang mở của một tiến trình cụ thể. Mỗi tiến trình có bảng riêng.

#### Open File Table:

- Bảng lưu trữ thông tin về tất cả các tệp đang mở trong hệ thống, chung cho tất cả tiến trình.

#### I-node Table:

- Bảng lưu trữ thông tin về các tệp, bao gồm quyền truy cập, kích thước, và vị trí dữ liệu trên đĩa.

## Synchronization và Asynchronization

### Synchronization (Đồng bộ)

- Đảm bảo truy cập an toàn tới tài nguyên chung.
- Sử dụng các cơ chế như mutexes và semaphores.
- Giúp tránh xung đột và deadlocks.

### Asynchronization( Không đồng bộ)

- Cho phép tiến trình thực hiện công việc mà không cần chờ đợi kết quả ngay lập tức.
- Tối ưu hóa hiệu suất, đặc biệt trong các tác vụ I/O.
- Sử dụng callback hoặc promises để xử lý kết quả khi hoàn tất.

## File Locking

### Giới thiệu về File Locking

File locking là một cơ chế quan trọng trong lập trình đa tiến trình, giúp đảm bảo tính nhất quán của dữ liệu khi nhiều tiến trình cùng truy cập vào một file.

### So Sánh Flock() và Fcntl()

| **Flock()**                                         | **Fcntl()**                                                   |
| --------------------------------------------------- | ------------------------------------------------------------- |
| Đơn giản                                            | Phức tạp                                                      |
| Thông tin ghi vào i-node là trạng thái lock         | Thông tin ghi vào i-node là trạng thái lock, tiến trình lock  |
| Lock toàn bộ file                                   | Lock được từng khu vực của file                               |
| Tại một thời điểm chỉ một tiến trình đọc/ghi 1 file | Nhiều tiến trình có thể đọc/ghi cùng 1 file mà không xung đột |

### Cú Pháp Sử Dụng Flock()

```c
int flock(int fd, int operation);
```

fd: File descriptor của file cần lock.

operation: Giá trị lock muốn set.

- LOCK_SH: set read lock.
- LOCK_EX: set write lock.
- LOCK_UN: unlock.

### Cú Pháp Sử Dụng Fcntl()

```c
int fcntl(int fd, int cmd, ... /* arg */ );
```

fd: File descriptor của file cần lock.

cmd: Action để thực hiện (LOCK_SH, LOCK_EX, LOCK_UN).

arg: Cấu trúc flock để mô tả lock cần thiết.

### Kết Luận

File locking là một công cụ hữu ích trong lập trình đa tiến trình, giúp bảo vệ dữ liệu và đảm bảo tính nhất quán khi nhiều tiến trình truy cập vào cùng một file.
