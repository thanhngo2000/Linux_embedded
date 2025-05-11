# Socket

## 1. Định nghĩa

- Socket là cơ chế truyền thông cho phép các tiến trình có thể giao tiếp với nhau dù các tiến tiến trình ở trên cùng 1 thiết bị hay khác thiết bị

## 2. Phân loại

### Domain Socket

- (còn được gọi là local socket hoặc UNIX domain socket) cho phép giao tiếp giữa các tiến trình thông qua một file socket trên hệ thống tệp.
- Một tiến trình sẽ tạo một socket, liên kết nó với một file socket, và sau đó các tiến trình khác có thể kết nối đến file đó.
- Giao tiếp qua domain socket thường nhanh hơn so với giao tiếp qua mạng TCP/IP, vì không có overhead của mạng.
- Vì giao tiếp chỉ diễn ra trên cùng một máy, nên có thể sử dụng các quyền truy cập file để kiểm soát ai có thể kết nối đến socket.
  **Domain Socket được chia là 2 loại**

#### Stream Socket

- Còn được gọi là TCP socket, sử dụng giao thức TCP (Transmission Control Protocol).
- Cung cấp một kết nối đáng tin cậy và hướng kết nối giữa hai tiến trình.
- Cần thiết lập một kết nối giữa hai bên trước khi truyền dữ liệu (sử dụng connect() và bind()).- Thời gian trễ thấp trong việc truyền tải dữ liệu lớn.

#### Datagram Socket

- Còn được gọi là UDP socket, sử dụng giao thức UDP (User Datagram Protocol).
- Cung cấp dịch vụ không kết nối, cho phép gửi và nhận dữ liệu mà không cần thiết lập một kết nối.
- Không đảm bảo độ tin cậy, thứ tự hoặc tính toàn vẹn của dữ liệu.
- Thích hợp cho các ứng dụng cần truyền tải dữ liệu nhanh mà không cần kiểm tra lỗi, như video streaming hoặc trò chơi trực tuyến.

### Internet Socket

- Giao tiếp giữa các tiến trình năm trên thiết bị khác nhau

## 3. Little Endian và Big Endian

### Little Endian

- Trong hệ thống little-endian, byte thấp (least significant byte - LSB) được lưu trữ trước tiên, tức là ở địa chỉ bộ nhớ thấp hơn
- Ví dụ: Số nguyên 0x12345678 sẽ được lưu trữ

```bash
Địa chỉ bộ nhớ:   0x00  0x01  0x02  0x03
Giá trị:          0x78  0x56  0x34  0x12
```

### Big Endian

- Trong hệ thống big-endian, byte cao (most significant byte - MSB) được lưu trữ trước tiên, tức là ở địa chỉ bộ nhớ thấp hơn
- Ví dụ: Số nguyên 0x12345678 sẽ được lưu trữ trong bộ nhớ như sau:

```bash
Địa chỉ bộ nhớ:   0x00  0x01  0x02  0x03
Giá trị:          0x12  0x34  0x56  0x78
```

### Chuyển Đổi từ Little-Endian sang Big-Endian

- htonl(): Chuyển đổi một số nguyên 32-bit từ host byte order (có thể là little-endian) sang network byte order (big-endian).
- htons(): Chuyển đổi một số nguyên 16-bit từ host byte order sang network byte order.

### Chuyển đổi từ Big-Endian sang Little-Endian

- ntohl(): Chuyển đổi một số nguyên 32-bit từ network byte order (big-endian) về host byte order.
- ntohs(): Chuyển đổi một số nguyên 16-bit từ network byte order về host byte order.
