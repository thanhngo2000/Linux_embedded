#ifndef LOGGER_H
#define LOGGER_H
#include "../../include/shared_data.h"

void create_fifo_file(const char *fifo_file_name);
void create_log_file(const char *log_file_name);
void init_log_manager();
void cleanup_log_manager();
void read_log_file(const char *log_file_path);
int clear_log_file(const char *log_file_name);
void *log_manager(void *arg);

#endif