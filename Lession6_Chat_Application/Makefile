# Định nghĩa biến
CC = gcc
CFLAGS = -Iinclude -Wall
LDFLAGS = -lpthread   # Cờ liên kết đúng
TARGET = app          # Tên file thực thi

# Danh sách file nguồn
SRC = src/client/client.c \
      src/connection/connection.c \
      src/server/server.c \
      src/user_interface/user_interface.c \
      src/utils/utils.c \
      src/main.c

# Quy tắc chính
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Quy tắc xóa file biên dịch
clean:
	rm -f $(TARGET)

.PHONY: clean