//
// Created by sims0702 on 12/27/23.
//

#include "datamgr.h"
dplist_t *sensor_list;

/* Definition of element_t */
typedef struct {
    int room_id;
    int sensor_id;
    sensor_value_t sum;
    sensor_value_t running_avg;
    int temp_count;
    sensor_ts_t last_timestamp;

} sensor_element_t;

// dplist functions
void *element_copy(void *element)
{
    sensor_element_t *copy = malloc(sizeof(sensor_element_t) * RUNNING_AVG_LENGTH);
    ERROR_HANDLER(copy == NULL, EXIT_MEMORY_ALLOCATION_ERROR, "memory allocation failed");
    copy->room_id = ((sensor_element_t *)element)->room_id;
    copy->sensor_id = ((sensor_element_t *)element)->sensor_id;
    copy->sum =  ((sensor_element_t*)element)->sum;
    copy->running_avg = ((sensor_element_t*)element)->running_avg;
    copy->last_timestamp = ((sensor_element_t*)element)->last_timestamp;
    return (void *)copy;
}

void element_free(void **element)
{
    free(*element);
    *element = NULL;
}

int element_compare(void *x, void *y)
{
    return ((((sensor_element_t *)x)->room_id < ((sensor_element_t *)y)->room_id) ? -1 : (((sensor_element_t *)x)->room_id == ((sensor_element_t *)y)->room_id) ? 0
                                                                                                                                                                : 1);
}



// datamgr functions

void datamgr_parse_sensor_mapping(FILE *fp_sensor_map) {
    /* sensor list creation*/
    sensor_list = dpl_create(element_copy, element_free, element_compare);

    /* parsing of room to sensor mappings */
    int counter = 0;
    while (1){
        sensor_element_t *temp_node = (sensor_element_t*)malloc(sizeof(sensor_element_t));
        ERROR_HANDLER(temp_node == NULL, EXIT_MEMORY_ALLOCATION_ERROR, "Memory allocation failed");


        int items_read = fscanf(fp_sensor_map, "%d %d\n", &(temp_node->room_id), &(temp_node->sensor_id));
        if (items_read == 2) { // reading sensor id and room id
            ERROR_HANDLER(dpl_insert_at_index(sensor_list, temp_node, counter++, true) == NULL,
                          EXIT_DPLIST_ERROR, "Error during insertion node in list"); // copy necessary as I use same node.
            free(temp_node);
        } else if (items_read == EOF){ // end of file
            free(temp_node);
            break;
        } else { // error occurred
            free(temp_node);
            ERROR_HANDLER(1, EXIT_FILE_ERROR, "Error occurred during reading from file");
        };
    }

    sensor_element_t *sensor_node;
    for (int i = 0; i < dpl_size(sensor_list); ++i) {
        sensor_node = (sensor_element_t*)dpl_get_element_at_index(sensor_list, i); // no copy returned
        ERROR_HANDLER(sensor_node == NULL, EXIT_DPLIST_ERROR, "Error: sensor list is empty, please add sensor_room mappings and try again.");
        for (int j = 0; j < RUNNING_AVG_LENGTH; j++) {
            sensor_node->temp_count = 0;
            sensor_node->sum = 0.0;
            sensor_node->running_avg = 0.0;
        }
    }

    for (int i = 0; i < dpl_size(sensor_list); ++i) {
        sensor_element_t *sensor_nod;
        sensor_nod = (sensor_element_t*)dpl_get_element_at_index(sensor_list, i);
        ERROR_HANDLER(sensor_nod == NULL, EXIT_DPLIST_ERROR, "Error: sensor list is empty, please add sensor_room mappings and try again.");

    }
}


void datamgr_update_sensor_data(sensor_data_t *sensor_data) {
    int num_of_sensors = dpl_size(sensor_list);
    ERROR_HANDLER(num_of_sensors == -1, 1, "Error: No sensors are present at this point of time.");

    // sensor data from shared buffer
    sensor_id_t sensor_id = sensor_data->id;
    sensor_value_t sensor_value = sensor_data->value;
    sensor_ts_t sensor_ts = sensor_data->ts;

    // checking invalid sensor id
    if (datamgr_is_invalid_sensor(sensor_id)) {
        LOG_MESSAGE("Received sensor data with invalid node ID %d. Connection will be closed shortly..", sensor_id);
        return;
    }

    // processing sensor data and determine whether room is too hot or too cold.
    for (int i = 0; i < num_of_sensors; i++) {
        sensor_element_t* sensor_node = (sensor_element_t*) dpl_get_element_at_index(sensor_list, i);
        ERROR_HANDLER(sensor_node == NULL, EXIT_DPLIST_ERROR, "Error: sensor list is empty, please add sensor_room mappings and try again.");

        if (sensor_node->sensor_id == sensor_id) {
            sensor_node->last_timestamp = sensor_ts;
            if (sensor_node->temp_count == RUNNING_AVG_LENGTH) { // use of formula determining new running avg by removing oldest val by newest val
                sensor_value_t old_running_avg = sensor_node->running_avg;
                sensor_node->running_avg = old_running_avg + (sensor_value - old_running_avg) / RUNNING_AVG_LENGTH;
                // logging only starts after having 5 values to determine avg temp.
                datamgr_avg_temp_logging(sensor_node->room_id, sensor_node->sensor_id, sensor_node->running_avg);
            } else { // if count not yet reached. this way accurate avg will be achieved
                sensor_node->temp_count++;
                sensor_node->sum += sensor_value;
                sensor_node->running_avg = 1.0* sensor_node->sum / sensor_node->temp_count;
            }
//            printf("%d sensor id: %d -> running average equals: %f and from function: %f\n", sensor_node->temp_count,sensor_node->sensor_id, sensor_node->running_avg, datamgr_get_avg(sensor_node->sensor_id));
        }
    }
}

void datamgr_avg_temp_logging(int room_id, int sensor_id, sensor_value_t avg_temp){
    if (avg_temp > SET_MAX_TEMP) {
        LOG_MESSAGE("Room %d using sensor node %d is too hot.(avg temp = %.2f)", room_id, sensor_id, avg_temp);
    }
    else if (avg_temp < SET_MIN_TEMP) {
        LOG_MESSAGE("Room %d using sensor node %d is too cold.(avg temp = %.2f)", room_id, sensor_id, avg_temp);
    }
}
void datamgr_free() {
    dpl_free(&sensor_list, true);
}

uint16_t datamgr_get_room_id(sensor_id_t sensor_id) {
    for (int i = 0; i < dpl_size(sensor_list); i++) { // data will already have been parsed, so no need to check dpl_size for -1 i.e. being empty
        sensor_element_t *sensor_node = dpl_get_element_at_index(sensor_list, i);
        ERROR_HANDLER(sensor_node == NULL, EXIT_DPLIST_ERROR, "Error: sensor list is empty, please add sensor_room mappings and try again.");
        if (sensor_id == sensor_node->sensor_id) {
            return sensor_node->room_id;
        }
    }
    ERROR_HANDLER(1, -1, "Sensor id doesn't exist.");
}

sensor_value_t datamgr_get_avg(sensor_id_t sensor_id) {
    sensor_value_t avg_temp;
    sensor_element_t *sensor;

    for (int i = 0; i < dpl_size(sensor_list); i++) {
        sensor_element_t *sensor_node = dpl_get_element_at_index(sensor_list, i);
        ERROR_HANDLER(sensor_node == NULL, EXIT_DPLIST_ERROR, "Error: sensor list is empty, please add sensor_room mappings and try again.");
        if (sensor_id == sensor_node->sensor_id) {
            sensor = dpl_get_element_at_index(sensor_list, i);
            break;
        }
        ERROR_HANDLER(i == dpl_size(sensor_list) - 1, -1, "Sensor id doesn't exist. avg can't be calculated.");
    }

    if (sensor->temp_count < RUNNING_AVG_LENGTH) { // checking only the last value is enough!
        return 0;
    } else {
        return sensor->running_avg;
    }


    // checking if temp dev is bigger or smaller than temp dev for correction.
    if (avg_temp > SET_MAX_TEMP) {
        fprintf(stderr, "The room is too hot.\n");
    }
    else if (avg_temp < SET_MIN_TEMP) fprintf(stderr, "The room is too cold.\n");
    return avg_temp;
}

time_t datamgr_get_last_modified(sensor_id_t sensor_id) {
    sensor_element_t *sensor_node;
    for (int i = 0; i < dpl_size(sensor_list); ++i) {
        sensor_node = dpl_get_element_at_index(sensor_list, i);
        ERROR_HANDLER(sensor_node == NULL, EXIT_DPLIST_ERROR, "Error: sensor list is empty, please add sensor_room mappings and try again.");
        if (sensor_id == sensor_node->sensor_id) {
            return sensor_node->last_timestamp;
        }
    }
    ERROR_HANDLER(1, -1, "Sensor id doesn't exist.");
}

int datamgr_get_total_sensors() {
    return dpl_size(sensor_list);
}

bool datamgr_is_invalid_sensor(sensor_id_t sensor_id) {
    sensor_element_t *sensor_node;
    for (int i = 0; i < dpl_size(sensor_list); i++) {
        sensor_node = (sensor_element_t*) dpl_get_element_at_index(sensor_list, i);
        ERROR_HANDLER(sensor_node == NULL, EXIT_DPLIST_ERROR, "Error: sensor list is empty, please add sensor_room mappings and try again.");
        if (sensor_id == sensor_node->sensor_id) return false;
    }
    return true;
}