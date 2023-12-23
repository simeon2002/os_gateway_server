#ifndef OS_PROJECT_CONNECTION_MGR_H
#define OS_PROJECT_CONNECTION_MGR_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <inttypes.h>
#include "config.h"
#include "lib/tcpsock.h"

#define EXIT_TCP_ERROR 1
#define EXIT_THREAD_ERROR 2
/*
 * Use ERROR_HANDLER() for handling memory allocation problems, invalid sensor IDs, non-existing files, etc.
 */
#define ERROR_HANDLER(condition, exit_status, ...)    do {                       \
                      if (condition) {                              \
                        fprintf(stderr, "\nError: in %s - function %s at line %d: %s\n", __FILE__, __func__, __LINE__, __VA_ARGS__); \
                        exit(exit_status);                         \
                      }                                             \
                    } while(0)

#endif //OS_PROJECT_CONNECTION_MGR_H


void *client_handler(tcpsock_t *client);

int cmgr_start_server(int argc, char** argv);