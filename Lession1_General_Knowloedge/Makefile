# create directory
CUR_DIR := .
BIN_DIR := $(CUR_DIR)/bin
INC_DIR := $(CUR_DIR)/inc
LIB_DIR := $(CUR_DIR)/lib
OBJ_DIR := $(CUR_DIR)/obj
SRC_DIR := $(CUR_DIR)/src
LIB_STATIC_DIR := $(LIB_DIR)/static_lib
LIB_SHARED_DIR := $(LIB_DIR)/shared_lib

LDFLAGS := -Wl,-rpath,$(LIB_SHARED_DIR) #help find shared libraries

#create objects
create_objs:
	gcc -c -fPIC $(SRC_DIR)/hellothanh.c -o $(OBJ_DIR)/hellothanh.o -I $(INC_DIR)
	gcc -c -fPIC $(SRC_DIR)/helloworld.c -o $(OBJ_DIR)/helloworld.o -I $(INC_DIR)
	gcc -c -fPIC $(SRC_DIR)/main.c -o $(OBJ_DIR)/main.o -I $(INC_DIR)

#create shared lib .so files
create_shared_lib:
	gcc -shared -o $(LIB_SHARED_DIR)/libhello.so $(OBJ_DIR)/hellothanh.o $(OBJ_DIR)/helloworld.o 

#create static lib .a files
create_static_lib:
	ar rcs -o $(LIB_STATIC_DIR)/libhello.a $(OBJ_DIR)/hellothanh.o $(OBJ_DIR)/helloworld.o  
#compile shared lib program
all_shared_lib: create_objs create_shared_lib
	gcc $(OBJ_DIR)/main.o -L $(LIB_SHARED_DIR) -lhello $(LDFLAGS) -o $(BIN_DIR)/use_shared_lib
#compile static lib program
all_static_lib: create_objs create_static_lib
	gcc $(OBJ_DIR)/main.o -L $(LIB_STATIC_DIR) -lhello -o $(BIN_DIR)/use_static_lib
#clean files
clean:
	rm -rf $(OBJ_DIR)/*.o
	rm -rf $(LIB_SHARED_DIR)/*.so
	rm -rf $(LIB_STATIC_DIR)/*.a
	rm -rf $(BIN_DIR)/use_shared_lib
	rm -rf $(BIN_DIR)/use_static_lib
