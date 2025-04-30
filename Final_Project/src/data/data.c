/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "data.h"
/******************************************************************************/
/*                            FUNCTIONS                              */
/******************************************************************************/
/**
 * \brief Calculates the running average of temperature readings.
 *
 * \param history Pointer to the TemperatureHistory structure containing the temperature data.
 *
 * \return float The calculated running average, or 0.0f if no data is available.
 */
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
/**
 * \brief Updates the temperature history for a given sensor.
 *
 * \param history Pointer to the TemperatureHistory structure for the sensor.
 * \param new_temp The new temperature reading to be added to the history.
 *
 * \note If the history size is reached, the new temperature will overwrite the oldest value.
 */
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
/**
 * \brief Checks the temperature status of a given sensor and logs warnings if thresholds are exceeded.
 *
 * \param sensor_id The ID of the sensor to check.
 * \param temperature The current temperature reading from the sensor.
 *
 * \return void
 *
 * \note This function checks if the average temperature of a sensor exceeds the hot or cold thresholds
 * and logs a warning if necessary. It also ensures thread safety by using a mutex.
 */
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
/**
 * \brief Initializes the data manager for handling sensor data.
 *
 * \return void
 *
 * \note This function sets up the mutex and initializes the thresholds and sensor history storage.
 */
void init_data_manager()
{
    pthread_mutex_init(&system_manager.data_manager.mutex, NULL);
    system_manager.data_manager.hot_threshold = 50.0f;
    system_manager.data_manager.cold_threshold = 10.0f;
    system_manager.data_manager.sensor_histories = NULL;
}
/**
 * \brief Cleans up the data manager by freeing allocated resources.
 *
 * \return void
 *
 * \note This function frees the memory for sensor histories and destroys the mutex to release resources.
 */
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
/**
 * \brief The main function for the data manager thread that processes sensor data periodically.
 *
 * \param arg A pointer to any arguments passed to the thread (not used here).
 *
 * \return void* Always returns NULL.
 *
 * \note This function continuously processes the data from active connections, checks the temperature status
 * of each sensor, and logs any warnings if thresholds are exceeded. The function sleeps for 1 second
 * between each iteration to regulate the processing rate.
 */
void *data_manager(void *arg)
{
    while (!stop_requested)
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
