# Message Queue

## 1. Khái niệm

- Message Queue là một cơ chế trong hệ điều hành Linux cho phép các tiến trình giao tiếp với nhau bằng cách gửi và nhận thông điệp.
- Là một cấu trúc dữ liệu cho phép lưu trữ thông điệp trong một hàng đợi (queue) theo thứ tự, giúp các tiến trình gửi và nhận thông điệp một cách không đồng bộ.
- Mỗi thông điệp trong hàng đợi có thể chứa một loại thông tin nhất định và có thể được xác định bằng một kiểu (type) nhất định.

## 2. Các bước triển khai

### SystemV Message Queue

**Tạo key**

- ftok(): được sử dụng để tạo một khóa duy nhất từ một đường dẫn tới tệp và một mã dự liệu.
  Tham số:
- const char \*pathname: Đường dẫn tới tệp.
- int proj_id: Mã dự liệu (thường là một ký tự).
  Giá trị trả về:
- Trả về một khóa (key_t) nếu thành công.
- Trả về -1 nếu thất bại và thiết lập errno.
  **Tạo message queue hoặc mở một message queue có sẵn**
- msgget() được sử dụng để tạo hoặc mở một message queue.
  Tham số:
- key_t key: Khóa của message queue.
- int msgflg: Cờ để xác định quyền truy cập (IPC_CREAT, IPC_EXCL, v.v.).
  Giá trị trả về:
- Trả về ID của message queue nếu thành công.
- Trả về -1 nếu thất bại và thiết lập errno.

**Ghi dữ liệu vào message Queue**

- Hàm msgsnd() được sử dụng để gửi một tin nhắn vào message queue.
  Tham số:
- int msqid: ID của message queue.
- const void \*msgp: Con trỏ đến cấu trúc tin nhắn.
- size_t msgsz: Kích thước của tin nhắn.
- int msgflg: Cờ để chỉ định hành vi (0 để gửi ngay).
  Giá trị trả về:
- Trả về 0 nếu thành công.
- Trả về -1 nếu thất bại và thiết lập errno.
  **Đọc dữ liệu từ message Queue**
- Hàm msgrcv() được sử dụng để nhận một tin nhắn từ message queue.
  Tham số:
- int msqid: ID của message queue.
- void \*msgp: Con trỏ đến cấu trúc để lưu trữ tin nhắn nhận được.
- size_t msgsz: Kích thước tối đa của tin nhắn.
- long msgtyp: Loại tin nhắn cần nhận (0 để nhận mọi loại).
- int msgflg: Cờ chỉ định hành vi (0 hoặc IPC_NOWAIT).
  Giá trị trả về:
- Trả về kích thước của tin nhắn nhận được nếu thành công.
- Trả về -1 nếu thất bại và thiết lập errno.
  **giải phóng message queue**
- Hàm msgctl() được sử dụng để thực hiện các thao tác điều khiển trên message queue, như xóa queue hoặc lấy thông tin về queue.
  Tham số:
- int msqid: ID của message queue.
- int cmd: Lệnh (IPC_STAT, IPC_RMID, v.v.).
- struct msqid_ds \*buf: Con trỏ đến cấu trúc để lưu thông tin (nếu cần).
  Giá trị trả về:
- Trả về 0 nếu thành công.
- Trả về -1 nếu thất bại và thiết lập errno.

### POSIX Message Queues

**Open mesage**

- mq_open: Hàm này cho phép bạn mở một hàng đợi tin nhắn đã tồn tại hoặc tạo một hàng đợi mới nếu nó chưa tồn tại.
- Sau khi mở, bạn có thể gửi và nhận tin nhắn từ hàng đợi đó.

```copy
mqd_t mq_open(const char *name, int oflag, mode_t mode, struct mq_attr *attr);
```

**Sending message**

- mq_send()

```copy
int mq_send(mqd_t mqd, const char *msg_ptr, size_t msg_len, unsigned int msg_prio);
```

**Receive message**

- mq_receive()

```copy
ssize_t mq_receive(mqd_t mqd, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);
```

**Delete message**

- mq_unlink();

```copy
int mq_unlink(const char *name);
```
