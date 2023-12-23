//
// Created by sims0702 on 12/23/23.
//

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "connection_mgr.h"

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
    return NULL;
}

void *storage_mgr_routine() {
    printf("testing storage\n");
    return NULL;
}


int main(int argc, char *argv[]) {
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