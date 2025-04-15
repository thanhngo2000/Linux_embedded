/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "data.h"
/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
static float calculate_running_average(TemperatureHistory *history)
{
    if (history->count == 0)
        return 0.0f;

    float sum = 0.0f;
    for (int i = 0; i < history->count; i++)
    {
        sum += history->history[i];
    }
    return sum / history->count;
}

static void update_temperature_history(TemperatureHistory *history, float new_temp)
{
    if (history->count < TEMPERATURE_HISTORY_SIZE)
    {
        history->history[history->count++] = new_temp;
    }
    else
    {
        history->history[history->index] = new_temp;
        history->index = (history->index + 1) % TEMPERATURE_HISTORY_SIZE;
    }
}
static void check_temperature_status(int sensor_id, float temperature)
{
    pthread_mutex_lock(&system_manager.data_manager.mutex);

    // check sensorID
    if (sensor_id < 0 || sensor_id >= MAX_CONNECTIONS)
    {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "Received data with invalid sensor ID: %d",
                 sensor_id);
        system_manager.log_manager.log(&system_manager.log_manager,
                                       LOG_ERROR, "Data", msg);
        return;
    }

    // sensor histories create
    if (system_manager.data_manager.sensor_histories == NULL)
    {
        system_manager.data_manager.sensor_histories =
            calloc(MAX_CONNECTIONS, sizeof(TemperatureHistory));
    }

    TemperatureHistory *history = &system_manager.data_manager.sensor_histories[sensor_id];
    update_temperature_history(history, temperature);
    float avg = calculate_running_average(history);

    // check threadhold
    char msg[256];
    if (avg > system_manager.data_manager.hot_threshold)
    {
        snprintf(msg, sizeof(msg),
                 "Sensor %d reports overheating (avg temp: %.1f)",
                 sensor_id, avg);
        system_manager.log_manager.log(&system_manager.log_manager,
                                       LOG_WARNING, "Data", msg);
    }
    else if (avg < system_manager.data_manager.cold_threshold)
    {
        snprintf(msg, sizeof(msg),
                 "Sensor %d reports overcooling (avg temp: %.1f)",
                 sensor_id, avg);
        system_manager.log_manager.log(&system_manager.log_manager,
                                       LOG_WARNING, "Data", msg);
    }

    pthread_mutex_unlock(&system_manager.data_manager.mutex);
}

void init_data_manager()
{
    pthread_mutex_init(&system_manager.data_manager.mutex, NULL);
    system_manager.data_manager.hot_threshold = 50.0f;
    system_manager.data_manager.cold_threshold = 10.0f;
    system_manager.data_manager.sensor_histories = NULL;
}
void cleanup_data_manager()
{
    pthread_mutex_lock(&system_manager.data_manager.mutex);
    if (system_manager.data_manager.sensor_histories != NULL)
    {
        free(system_manager.data_manager.sensor_histories);
        system_manager.data_manager.sensor_histories = NULL;
    }
    pthread_mutex_unlock(&system_manager.data_manager.mutex);
    pthread_mutex_destroy(&system_manager.data_manager.mutex);
}
// data manager
void *data_manager(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&system_manager.connection_manager.mutex);

        ConnectionNode *current = system_manager.connection_manager.head;
        while (current != NULL)
        {
            if (current->connection.is_active && current->latest_data.is_valid)
            {
                check_temperature_status(
                    current->connection.sensor_id,
                    current->latest_data.temperature);
            }
            current = current->next;
        }

        pthread_mutex_unlock(&system_manager.connection_manager.mutex);
        sleep(1);
    }
    return NULL;
}
