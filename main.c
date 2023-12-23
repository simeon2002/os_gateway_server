//
// Created by sims0702 on 12/23/23.
//

// note: to define a global var, define it in the head file of the appropriate module as extern in .h file, then define it
// in the .c file of that module and just include the header file in the files that you want to use the global variable basically.
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
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
    return NULL;
}

void *data_mgr_routine() {
    printf("testing data\n");
    do {
        sensor_data_t *sensor_node = (sensor_data_t *) malloc(sizeof(sensor_data_t));
        if (sbuffer_remove(shared_buffer, sensor_node) == SBUFFER_NO_DATA) break;
        printf("Testing out shared buffer:\n");
        printf("sensor id: %d, %f, %ld\n", sensor_node->id, sensor_node->value, sensor_node->ts);
    } while (1);
    return NULL;
}

void *storage_mgr_routine() {
    printf("testing storage\n");
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

    for (int i = 0; i < NUM_OF_THREADS; i++) {
        if (pthread_join(*(threads+i), NULL) != 0) {
            ERROR_HANDLER(1, EXIT_THREAD_ERROR, "Error occurred during thread join");
        }
    }

    free(args);
    return 0;
}