/**
 * \author {AUTHOR}
 */

#include <stdlib.h>
#include <stdio.h>
#include "sbuffer.h"
#include <semaphore.h>

/**
 * basic node for the buffer, these nodes are linked together to create the buffer
 */
typedef struct sbuffer_node {
    struct sbuffer_node *next;  /**< a pointer to the next node*/
    sensor_data_t data;         /**< a structure containing the data */
} sbuffer_node_t;

/**
 * a structure to keep track of the buffer
 */
struct sbuffer {
    sbuffer_node_t *head;       /**< a pointer to the first node in the buffer */
    sbuffer_node_t *tail;       /**< a pointer to the last node in the buffer */
};

sem_t rw_mutex;
sem_t buffer_count;
sbuffer_t *shared_buffer;

int sbuffer_init(sbuffer_t **buffer) {
    if (sem_init(&buffer_count, 0, 0) != 0) {
        perror("buffer_count semaphore cannot be created");
        return SBUFFER_FAILURE;
    }
    if (sem_init(&rw_mutex, 0, 1) != 0) {
        perror("rw_mutex semaphore cannot be created");
        return SBUFFER_FAILURE;
    }

    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    return SBUFFER_SUCCESS;
}

int sbuffer_free(sbuffer_t **buffer) {
    sbuffer_node_t *dummy;
    if ((buffer == NULL) || (*buffer == NULL)) {
        return SBUFFER_FAILURE;
    }
    while ((*buffer)->head) {
        dummy = (*buffer)->head;
        (*buffer)->head = (*buffer)->head->next;
        free(dummy);
    }
    free(*buffer);
    *buffer = NULL;
    if (sem_destroy(&rw_mutex) != 0) {
        perror("rw_mutex semaphore cannot be destroyed");
    }
    if (sem_destroy(&buffer_count) != 0) {
        perror("buffer_count semaphore cannot be destroyed");
    }
    return SBUFFER_SUCCESS;
}


int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data) {
    sbuffer_node_t *dummy;
    // failure case
    if (buffer == NULL) return SBUFFER_FAILURE;
    if (sem_wait(&buffer_count) != 0){ // waits to get something in the buffer.
        perror("Error occurred during waiting for counting semaphore");
        return SBUFFER_FAILURE;
    }
    // no data case (not utilized in my code as this is accounted for by waiting semaphore)
//    if (buffer->head == NULL) return SBUFFER_NO_DATA; // TODO QUESTION: there is not really a need for that one in my case?
    // data removed cases
    if (sem_wait(&rw_mutex) != 0){ // start buffer lock
        perror("Error occurred during waiting for semaphore mutex");
        return SBUFFER_FAILURE;
    }
    *data = buffer->head->data;
    dummy = buffer->head;
    if (buffer->head == buffer->tail) // buffer has only one node
    {
        buffer->head = buffer->tail = NULL;
    } else  // buffer has many nodes empty
    {
        buffer->head = buffer->head->next;
    }

    if (data->id == 0) { // checking for end of file marker
        sem_post(&rw_mutex);
        return SBUFFER_NO_DATA;
    }

    if (sem_post(&rw_mutex) != 0) { // end buffer lock
        perror("Error occurred during release of semaphore mutex");
        return SBUFFER_FAILURE;
    }
    free(dummy);
    return SBUFFER_SUCCESS;
}

int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    sbuffer_node_t *dummy;
    dummy = (sbuffer_node_t*) malloc(sizeof(sbuffer_node_t));
    if (buffer == NULL) return SBUFFER_FAILURE;
    if (dummy == NULL) return SBUFFER_FAILURE;
    dummy->data = *data;
    dummy->next = NULL;
    if (buffer->tail == NULL) // buffer was empty: no semaphore needed here as we have buffer_counter blocking the reader already.
    {
        buffer->head = buffer->tail = dummy;
    } else // buffer not empty
    {
        if (sem_wait(&rw_mutex) != 0){ // lock can start here, because reader can't access buffer it iis empty anyways.
            perror("Error occurred during waiting for semaphore mutex");
            return SBUFFER_FAILURE;
        }
        buffer->tail->next = dummy;
        buffer->tail = buffer->tail->next;
        if (sem_post(&rw_mutex) != 0) { // end buffer lock
            perror("Error occurred during release of semaphore mutex");
            return SBUFFER_FAILURE;
        }
    }
    if (sem_post(&buffer_count) != 0){ // signals that there is something in buffer.
        perror("Error occurred during signaling of counting semaphore");
        return SBUFFER_FAILURE;
        }
    return SBUFFER_SUCCESS;
}
