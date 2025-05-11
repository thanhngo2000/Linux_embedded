# Định nghĩa biến
CC = gcc
CFLAGS = -Iinclude -Wall
LDFLAGS = -lpthread   # Cờ liên kết đúng
TARGET = app          # Tên file thực thi

# Danh sách file nguồn
SRC = src/connection/connection.c \
      src/user_interface/user_interface.c \
      src/utils/utils.c \
      src/socket/socket.c \
      src/message/message.c \
      src/thread/thread.c \
      src/logger/logger.c \
      src/main.c

# Quy tắc chính
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Quy tắc xóa file biên dịch
clean:
	rm -f $(TARGET)

.PHONY: clean