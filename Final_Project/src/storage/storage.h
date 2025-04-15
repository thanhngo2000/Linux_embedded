#ifndef STORAGE_H
#define STORAGE_H

#include "../../include/shared_data.h"
#include "../logger/logger.h"

#define SQL_CREATE_TABLE "CREATE TABLE IF NOT EXISTS sensor_data (id INTEGER PRIMARY KEY, timestamp INTEGER, sensor_id INTEGER, temperature REAL, is_valid INTEGER)"
void init_storage_manager();
void cleanup_storage_manager();
void storage_add_data(SensorData data);
void storage_print_all_data();
void *storage_manager(void *arg);

#endif