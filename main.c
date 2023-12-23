//
// Created by sims0702 on 12/23/23.
//

// note: to define a global var, define it in the head file of the appropriate module as extern in .h file, then define it
// in the .c file of that module and just include the header file in the files that you want to use the global variable basically.
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "connection_mgr.h"
#include "sbuffer.h"


#define NUM_OF_THREADS 3
#ifndef EXIT_THREAD_ERROR
#define EXIT_THREAD_ERROR 1
#endif

typedef struct {
    int argc;
    char **argv;
} thread_args;

void *connection_mgr_routine(void* arguments){
    thread_args *args= (thread_args*)arguments;
    int argc = args->argc;
    char **argv = args->argv;
    cmgr_start_server(argc, argv);
    fprintf(stdout, "closing the connection manager\n");
    fflush(stdout);
    return NULL;
}

void *data_mgr_routine() {
    printf("testing data\n");
    int counter = 0;
    sensor_data_t *sensor_node = (sensor_data_t *) malloc(sizeof(sensor_data_t));
    do {
        int result;
        result = sbuffer_remove(shared_buffer, sensor_node, true);
        if (result == SBUFFER_NO_DATA) break;
        else if (result == SBUFFER_NO_MATCH) continue;
        else {
            printf("DATAMGR:");
            counter++;
            printf("counter: %d sensor id: %d, %f, %ld\n", counter, sensor_node->id, sensor_node->value, sensor_node->ts);
            fflush(stdout);
        }
//        sleep(2);
    } while (1);
    free(sensor_node);
    printf("closing data manger\n");
    return NULL;
}

void *storage_mgr_routine() {
    int count = 0;
    printf("testing storage\n");
    sensor_data_t *sensor_node = (sensor_data_t *) malloc(sizeof(sensor_data_t));
    int result;
    do {
        result = sbuffer_remove(shared_buffer, sensor_node, false);
        if ( result == SBUFFER_NO_DATA) break;
        else if (result == SBUFFER_NO_MATCH) continue;
        else {
            printf("STORAGEMGR: ");
            count++;
            printf("count %d sensor id: %d, %f, %ld\n", count, sensor_node->id, sensor_node->value, sensor_node->ts);
            fflush(stdout);
        }
//        sleep(2);
    } while (1);
    free(sensor_node);
    printf("closing storage manager\n");
    return NULL;
}


int main(int argc, char *argv[]) {

    // initializing buffer
    sbuffer_init(&shared_buffer);

    // creating threads for each manager
    pthread_t threads[NUM_OF_THREADS];
    thread_args *args = (thread_args*)malloc(sizeof(thread_args));
    args->argc = argc;
    args->argv = argv;

    for (int i = 0; i < NUM_OF_THREADS; i++) {
        if (i == 0) {

            if (pthread_create(threads+i, NULL, (void*)connection_mgr_routine, (void*)args ) != 0) {
                ERROR_HANDLER(1, EXIT_THREAD_ERROR, "Error occurred during thread creation");
            }
        } else if ( i == 1) {
            if (pthread_create(threads+i, NULL, (void*)data_mgr_routine, NULL) != 0) {
                ERROR_HANDLER(1, EXIT_THREAD_ERROR, "Error occurred during thread creation");
            }
        } else {
            if (pthread_create(threads+i, NULL, (void*)storage_mgr_routine, NULL) != 0) {
                ERROR_HANDLER(1, EXIT_THREAD_ERROR, "Error occurred during thread creation");
            }
        }
    }

    // waiting for threads to finish
    for (int i = 0; i < NUM_OF_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Error joining thread.\n");
            exit(EXIT_FAILURE);
        }
    }

    free(args);
    return 0;
}