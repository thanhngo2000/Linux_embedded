# General Knowledge

# Description

Tóm tắt kiến thức của Lesson 1 embedded linux

## Bộ BSP (Board Support Package)

Là một tập hợp các tài nguyên cần thiết để hỗ trợ phần cứng cụ thể trong một hệ điều hành, thường là Linux. Nó cung cấp các driver, cấu hình và các phần mềm cần thiết để hoạt động với phần cứng đó.

### Bootloader

Là phần mềm được chạy đầu tiên khi thiết bị khởi động. Nó có nhiệm vụ nạp hệ điều hành vào bộ nhớ và khởi động nó. Ví dụ phổ biến là U-Boot hoặc GRUB.

### Linux Kernel

Là lõi của hệ điều hành Linux, quản lý tài nguyên phần cứng và cung cấp dịch vụ cho các ứng dụng. Kernel xử lý các thao tác với phần cứng, quản lý bộ nhớ, và thực hiện các tác vụ khác.

### Rootfs

Là hệ thống tệp gốc (root filesystem) chứa tất cả các tệp cần thiết cho hệ điều hành hoạt động. Nó bao gồm các thư mục hệ thống như /bin, /etc, /lib, và các tệp cấu hình.

### Toolchain

Là bộ công cụ phát triển phần mềm, bao gồm trình biên dịch (compiler), trình liên kết (linker), và các công cụ khác cần thiết để biên dịch và xây dựng phần mềm cho thiết bị nhúng.

## Cách chạy trình biên dịch gcc trên cmd

```bash
  gcc –o <tên thư mục tạo thành> <tên thư mục cần biên dịch>
```

## Build nhiều file cùng một lúc

```bash
  gcc –o <tên file compile ra> <tên các file cần build> -I <các file thư viện>
```

## **sudo apt install** và **apt install** có sự khác biệt chính như sau:

### Quyền Truy Cập

**apt install**: Khi bạn sử dụng lệnh này mà không có quyền sudo, lệnh sẽ không thực hiện được nếu bạn không có quyền truy cập root. Hệ thống sẽ thông báo lỗi về quyền truy cập.

**sudo apt install**: Lệnh này cho phép bạn thực hiện cài đặt với quyền root, nghĩa là bạn có khả năng thay đổi hệ thống, cài đặt hoặc gỡ bỏ phần mềm. sudo (SuperUser DO) yêu cầu bạn nhập mật khẩu của người dùng hiện tại để xác nhận quyền truy cập.

### Mục Đích Sử Dụng

**apt install**: Thường được sử dụng trong các script hoặc khi bạn đã có quyền root và không cần nhập mật khẩu.

**sudo apt install**: Dùng trong các tình huống mà bạn cần cài đặt phần mềm hoặc cập nhật hệ thống, và bạn không phải là người dùng root mặc định.

## Cách xem địa chỉ IP

### Window

```bash
  ipconfig
```

### Linux

```bash
  ifconfig
```

## Makefile

### Makefile là gì?

**Make file** là một script bên trong có chứa các thông tin:

- Cấu trúc của một project (file, dependency).
- Các command line dùng để tạo-hủy file.
  Chương trình make sẽ đọc nội dung trong Makefile và thực thi nó

### Những option trong makefile

- f File: Sử dụng tệp chỉ định (File) như một tệp makefile.
- i: Bỏ qua tất cả lỗi trong các lệnh được thực thi để làm mới các tệp.
- n: In ra các lệnh mà sẽ được thực thi, nhưng không thực thi chúng thực sự.

### Target trong makefile

- all: Thực hiện tất cả các mục tiêu cấp cao nhất trong makefile.
- info: Cung cấp thông tin về chương trình.
- clean: Xóa tất cả các tệp mà thường được tạo ra bằng lệnh make.
- install: Sao chép tệp thực thi vào thư mục mà các lệnh thường tìm kiếm.
- uninstall: Thực hiện ngược lại với install, xóa tệp đã được cài đặt.
- tar: Tạo một tệp tar từ mã nguồn.
- test: Thực hiện các bài kiểm tra tự động trên chương trình.

### Các biến và cờ (flags) cần thiết khi dùng với makefile

**CFLAGS:**

- **I**: Bao gồm tất cả các tệp tiêu đề (.h) theo sau bởi đường dẫn.
- **Wall**: Bật tất cả các cảnh báo trong quá trình biên dịch.
- **DDEBUG**: Định nghĩa macro DEBUG để sử dụng trong mã nguồn.
- **g**: Bật chế độ gỡ lỗi trong trình biên dịch gcc.
  **LDFLAGS:**
- **L/lib**: Chỉ định đường dẫn đến thư viện.
- **l<name>**: Tên của thư viện cần liên kết (library).

### Phân biệt các phép gán trong Makefile

- **=** Tính toán giá trị mỗi khi biến được sử dụng.
-     **:=** Tính toán giá trị ngay lập tức tại thời điểm gán.
-     **?=** Gán giá trị chỉ nếu biến chưa được định nghĩa

### Các lệnh sử dụng các biến đặc biệt trong Makefile:

- **@echo $@**: Tên tệp đầu ra của quy tắc hiện tại.
- **@echo $<**: Tên tệp đầu vào đầu tiên trong quy tắc.
- **@echo $^**: Danh sách tất cả các tệp đầu vào trong quy tắc.

## Phân biệt hai tùy chọn phổ biến là -o và -c.

**gcc -o**

- Chức Năng: Tùy chọn -o được sử dụng để chỉ định tên cho tệp thực thi đầu ra sau khi liên kết các tệp đối tượng.
- Cách Sử Dụng:

```bash
gcc -o <tên_tệp_đầu_ra> <tên_tệp_nguồn.c>
```

- Ví dụ:

```bash
gcc -o my_program main.o utils.o
```

- Lệnh này sẽ liên kết main.o và utils.o, và tạo ra một tệp thực thi có tên là my_program.
  **gcc -c**
- Chức Năng: Tùy chọn -c được sử dụng để biên dịch mã nguồn thành tệp đối tượng mà không liên kết (link) chúng. Tệp đối tượng có phần mở rộng là .o.
- Cách Sử Dụng:

```bash
gcc -c <tên_tệp_nguồn.c>
```

- Ví Dụ:

```bash
gcc -c myprogram.c
```

- Lệnh này sẽ biên dịch myprogram.c và tạo ra tệp đối tượng myprogram.o.

## Static Lib And Shared Lib

| Properties              | Static Lib                                                                                | Shared Lib                                                                                                          |
| ----------------------- | ----------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------- |
| **Thời điểm Liên kết**  | Tất cả các module trong thư viện được sao chép vào file thực thi tại thời điểm biên dịch. | Thư viện động được sử dụng trong quá trình liên kết khi cả file thực thi và thư viện được nạp vào bộ nhớ (runtime). |
| **Kích thước**          | Sử dụng thư viện tĩnh tốn nhiều bộ nhớ hơn.                                               | Sử dụng thư viện động tốn ít bộ nhớ hơn vì chỉ có một bản sao trong bộ nhớ.                                         |
| **Thay đổi File Ngoài** | File thực thi cần được biên dịch lại khi có thay đổi trong thư viện tĩnh.                 | Không cần biên dịch lại file thực thi khi có thay đổi trong thư viện động.                                          |
| **Thời gian Thực thi**  | Mất nhiều thời gian hơn để thực thi.                                                      | Mất ít thời gian hơn để thực thi vì thư viện đã nằm sẵn trong bộ nhớ.                                               |

### Tóm tắt

- **Thư viện Tĩnh**: Liên kết tại thời điểm biên dịch, kích thước file thực thi lớn hơn, yêu cầu biên dịch lại khi thư viện thay đổi, và thời gian thực thi chậm hơn.
- **Thư viện Động**: Liên kết tại thời điểm chạy, tiết kiệm bộ nhớ hơn, không cần biên dịch lại khi thư viện thay đổi, và thời gian thực thi nhanh hơn.
