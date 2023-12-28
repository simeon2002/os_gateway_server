/**
 * \author {AUTHOR}
 */

#ifndef _SBUFFER_H_
#define _SBUFFER_H_

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdbool.h>
#include <math.h>

#define SBUFFER_FAILURE -1
#define SBUFFER_SUCCESS 0
#define SBUFFER_END 1
#define SBUFFER_NO_MATCH 2
#define SBUFFER_NO_DATA 3

typedef struct sbuffer sbuffer_t;
extern sbuffer_t *shared_buffer;

/**
 * Allocates and initializes a new shared buffer as well as necessary mutexes and semaphores
 * Uses ERROR_HANLDER() to handle errors.
 * \param buffer a double pointer to the buffer that needs to be initialized
 * \return returns SBUFFER_SUCCESS on success and exits with Status code on error */
int sbuffer_init(sbuffer_t **buffer);

/**
 * All allocated resources are freed and cleaned up
 * Uses ERROR_HANLDER() to handle errors.
 * \param buffer a double pointer to the buffer that needs to be freed
 * \return returns SBUFFER_SUCCESS on success and exits with Status code on error
 */
int sbuffer_free(sbuffer_t **buffer);

/**
 * Removes the first sensor data in 'buffer' (at the 'head') and returns this sensor data as '*data'
 * If 'buffer' is empty, the function doesn't block until new sensor data becomes available but returns SBUFFER_NO_DATA
 * if boolean value of is_datamgr doens't match the one stored in sensor_data struct, it returns SBUFFER_NO_MATCH
 * if sbuffer is the last value, SBUFFER_END will be returned
 * \param buffer a pointer to the buffer that is used
 * \param data a pointer to pre-allocated sensor_data_t space, the data will be copied into this structure. No new memory is allocated for 'data' in this function.
 * \param is_datamgr a boolean that determines which manager is removing data from buffer.
 * \return returns SBUFFER_SUCCESS on success and returns with SBUFFER_FAILURE on error
 */
int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data, bool is_datamgr);

/**
 * Inserts the sensor data in 'data' at the end of 'buffer' (at the 'tail')
 * \param buffer a pointer to the buffer that is used
 * \param data a pointer to sensor_data_t data, that will be copied into the buffer
 * \param release_lock a boolean flag, when true lock will be taken, when false lock will be released.
 * \return returns SBUFFER_SUCCESS on success and returns with SBUFFER_FAILURE on error
*/
int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data, bool release_lock);

#endif  //_SBUFFER_H_
