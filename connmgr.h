#ifndef OS_PROJECT_CONNECTION_MGR_H
#define OS_PROJECT_CONNECTION_MGR_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <inttypes.h>
#include "config.h"
#include "lib/tcpsock.h"
#include "sbuffer.h"
#include <unistd.h>
#include "logger.h"
#include <time.h>
#include <signal.h>
#include "datamgr.h"

#ifndef TIMEOUT
#define TIMEOUT 20
#endif

/**
 * This method handles the incoming data from the client via tcp connection. used for each thread.
 * It will receive sensor data and put that in the shared buffer twice, for the two different managers (storage and
 * data processing).
 * ERROR_HANDLER() is used to deal with errors.
 * \param client client's tcp socket, used for communication with client.
 * \return nothing
 * */
void *client_handler(tcpsock_t *client);

/**
 * This method starts up the server, listens for incoming clients and clients separate thread for each incoming client
 * until MAX nb of connections has been reached.
 * Errors are handled using ERROR_HANDLER().
 * \param argc nb of arguments received from cmd
 * \param argv list of arguments received from cmd
 * \return EXIT_SUCCESS on success, otherwise exit with appropriate status code.
 * */
int cmgr_server_startup(int argc, char** argv);

#endif //OS_PROJECT_CONNECTION_MGR_H
