//
// Created by sims0702 on 12/23/23.
//

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "connmgr.h"
#include "sbuffer.h"
#include "logger.h"
#include "datamgr.h"
#include "sensor_db.h"

#define NUM_OF_THREADS 3

typedef struct {
    int argc;
    char **argv;
} thread_args;

void *connection_mgr_routine(void* arguments){
    thread_args *cmd_args= (thread_args*)arguments;
    int argc = cmd_args->argc;
    char **argv = cmd_args->argv;
    cmgr_server_startup(argc, argv);
    return NULL;
}

void *data_mgr_routine() {
    // room sensor mappings
    FILE * map = fopen("room_sensor.map", "r");
    ERROR_HANDLER(map == NULL, EXIT_FILE_ERROR,  "Error: file couldn't open.");
    datamgr_parse_sensor_mapping(map);
    sensor_data_t *sensor_node = (sensor_data_t *) malloc(sizeof(sensor_data_t));
    ERROR_HANDLER(sensor_node == NULL, EXIT_MEMORY_ALLOCATION_ERROR, "Error: Memory couldn't be allocated.");
    int result;

    // reading data from buffer
    do {
        result = sbuffer_remove(shared_buffer, sensor_node, true);
        if (result == SBUFFER_END) break;
        else if (result == SBUFFER_NO_MATCH || result == SBUFFER_NO_DATA) continue;
        else if (result == SBUFFER_FAILURE) {
            ERROR_HANDLER(1, EXIT_BUFFER_ERROR, "Error during removal from buffer.");
        }
        else {
            /*updating sensor_nodes info based on data received from sbuffer*/
            datamgr_update_sensor_data(sensor_node);
        }

    } while (1);
    free(sensor_node);
    datamgr_free();
    fclose(map);
    return NULL;
}

void *storage_mgr_routine() {
    // opening db file
    char *filename = "data.csv";
    FILE* storage_f = storagemgr_open_db(filename, false);

    // shared buffer data retrieval
    sensor_data_t *sensor_node = (sensor_data_t *) malloc(sizeof(sensor_data_t));
    ERROR_HANDLER(sensor_node == NULL, EXIT_MEMORY_ALLOCATION_ERROR, "Error: Memory couldn't be allocated.");
    int result;
    do {
        result = sbuffer_remove(shared_buffer, sensor_node, false);
        if ( result == SBUFFER_END) break;
        else if (result == SBUFFER_NO_MATCH || result == SBUFFER_NO_DATA) continue;
        else if (result == SBUFFER_FAILURE) {
            ERROR_HANDLER(1, EXIT_BUFFER_ERROR, "Error during removal from buffer.");
        }
        else {
            if (datamgr_is_invalid_sensor(sensor_node->id)) { // checking for invalid sensor id
                LOG_MESSAGE("Data not added to %s file. Sensor data has invalid sensor id %d.", filename, sensor_node->id);
                continue;
            }
            storagemgr_insert_sensor(storage_f, sensor_node->id, sensor_node->value, sensor_node->ts);
        }
    } while (1);

    // cleaning up resources
    free(sensor_node);
    storagemgr_close_db(storage_f, filename);
    return NULL;
}


int main(int argc, char *argv[]) {
    // creation log process
    create_log_process();

    // initializing buffer
    sbuffer_init(&shared_buffer);

    // creating threads for each manager
    pthread_t threads[NUM_OF_THREADS];
    thread_args *args = (thread_args*)malloc(sizeof(thread_args));
    ERROR_HANDLER(args == NULL, EXIT_MEMORY_ALLOCATION_ERROR, "Error: Memory couldn't be allocated.");
    args->argc = argc;
    args->argv = argv;

    for (int i = 0; i < NUM_OF_THREADS; i++) {
        if (i == 0) {

            if (pthread_create(threads+i, NULL, (void*)connection_mgr_routine, (void*)args ) != 0) {
                ERROR_HANDLER(1, EXIT_THREAD_ERROR, "Error occurred during thread creation.");
            }
        } else if ( i == 1) {
            if (pthread_create(threads+i, NULL, (void*)data_mgr_routine, NULL) != 0) {
                ERROR_HANDLER(1, EXIT_THREAD_ERROR, "Error occurred during thread creation.");
            }
        } else {
            if (pthread_create(threads+i, NULL, (void*)storage_mgr_routine, NULL) != 0) {
                ERROR_HANDLER(1, EXIT_THREAD_ERROR, "Error occurred during thread creation.");
            }
        }
    }

    // waiting for threads to finish
    for (int i = 0; i < NUM_OF_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            ERROR_HANDLER(1, EXIT_THREAD_ERROR, "Error joining thread.");
        }
    }

    free(args);
    sbuffer_free(&shared_buffer);
    // ending log process.
    end_log_process();
    return 0;
}