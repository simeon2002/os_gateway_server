//
// Created by sims0702 on 12/27/23.
//

#ifndef OS_PROJECT_DATA_MGR_H
#define OS_PROJECT_DATA_MGR_H

#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "lib/dplist.h"
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/stat.h>

#ifndef RUNNING_AVG_LENGTH
/*Macro determines the length of the nb of last sensor values from which the average will be calculated.*/
#define RUNNING_AVG_LENGTH 5
#endif

#ifndef SET_MAX_TEMP
/*MAX temperature after which correction is necessary*/
#error SET_MAX_TEMP not set
#endif

#ifndef SET_MIN_TEMP
/*MIN temperature after which correction is necessary*/
#error SET_MIN_TEMP not set
#endif

/* defintion of the 3 user-defined functions. */
void *element_copy(void *element);
void element_free(void **element);
int element_compare(void *x, void *y);

/**
 * This method should be called to clean up the datamgr, and to free all used memory.
 * After this, any call to datamgr_get_room_id, datamgr_get_avg, datamgr_get_last_modified or datamgr_get_total_sensors will not return a valid result
 */
void datamgr_free();

/**
 * Gets the room ID for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the corresponding room id
 */
uint16_t datamgr_get_room_id(sensor_id_t sensor_id);

/**
 * Gets the running AVG of a certain senor ID (if less then RUN_AVG_LENGTH measurements are recorded the avg is 0)
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the running AVG of the given sensor
 */
sensor_value_t datamgr_get_avg(sensor_id_t sensor_id);

/**
 * Returns the time of the last reading for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the last modified timestamp for the given sensor
 */
time_t datamgr_get_last_modified(sensor_id_t sensor_id);

/**
 *  Return the total amount of unique sensor ID's recorded by the datamgr
 *  \return the total amount of sensors
 */
int datamgr_get_total_sensors();

/**
 * Logs message to log file if room is too hot or too cold. Function used internally.
 * \param room_id room id of sensor data
 * \param sensor_id sensor id of sensor data
 * \param avg_temp running average temperature at that point
 * */
void datamgr_avg_temp_logging(int room_id, int sensor_id, sensor_value_t avg_temp);

/**
 *  This method holds the core functionality of datamgr. It takes in a senser data element pointer and uses it to update
 *  the corresponding sensor node data.
 *  Uses ERROR_HANDLER() to deal with errors as well as appropriate messages will be logger if no error occurred.
 *  \param sensor_data sensor data struct containing id of sensor and more.
 */
void datamgr_update_sensor_data(sensor_data_t *sensor_data);

/**
 *  This method takes in 1 file pointer to the sensor_room mappings file and parses it.
 *  The method places the corresponding room id and sensor mappings into the list pointer.
 *  Uses ERROR_HANDLER() to deal with errors.
 *  \param fp_sensor_map file pointer to the map file
 */
void datamgr_parse_sensor_mapping(FILE *fp_sensor_map);

/**
 *  Return boolean value based on whether sensor id of the sensor data is in sensor list or not.
 *  Uses ERROR_HANDLER() to deal with errors.
 *  \param sensor_id sensor id of sensor data.
 *  \return false if sensor valid, true if sensor is invalid or exits with appropriate status code if error occurred
 */
bool datamgr_is_invalid_sensor(sensor_id_t sensor_id);

#endif //OS_PROJECT_DATA_MGR_H
