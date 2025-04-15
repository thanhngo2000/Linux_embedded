CC = gcc
CFLAGS = -Iinclude -Wall
LDFLAGS = -lpthread -lsqlite3 -lssl -lcrypto  # flag
TARGET = app          # execute file name
# source file list
SRC = src/connection/connection.c \
		src/data/data.c \
		src/logger/logger.c \
		src/storage/storage.c \
      src/user_interface/user_interface.c \
      src/utils/utils.c \
	  src/socket/socket.c\
	  src/security/security.c\
      src/main.c

#make
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# clean
clean:
	rm -rf $(TARGET) logFifo gateway.log sensor_data.db

.PHONY: clean