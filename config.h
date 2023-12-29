/**
 * \author {AUTHOR}
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <pthread.h>
#include <stdarg.h>

// DEFINITION ERROR CODES

#define EXIT_FORK_FAILURE 1
#define EXIT_LOG_ERROR 2
#define EXIT_PIPE_ERROR 3
#define EXIT_MUTEX_ERROR 4
#define EXIT_FILE_ERROR 5
#define EXIT_MEMORY_ALLOCATION_ERROR 6
#define EXIT_BUFFER_ERROR 7
#define EXIT_THREAD_ERROR 8
#define EXIT_DPLIST_ERROR 9
#define EXIT_TCP_ERROR 10
#define EXIT_CMD_ARGS_ERROR 11


#ifndef ERROR_HANDLER
/**
 * Use ERROR_HANDLER() for handling memory allocation problems, invalid sensor IDs, non-existing files, etc.
 * The method will display the error in CMD as well log message to log file.
 * \param condition condition that needs to be true to true for exit
 * \param exit_status status code to exit with.
 * \param ... used for message to be displayed.
 */
#define ERROR_HANDLER(condition, exit_status, ...)    do {                       \
                      if (condition) {                              \
                        printf("\nError: in %s - function %s at line %d: %s\n", __FILE__, __func__, __LINE__, __VA_ARGS__); \
                         LOG_MESSAGE("In %s - line %d: %s",                                         \
                    __FILE__, __LINE__, __VA_ARGS__);             \
                    exit(exit_status);                                           \
                      }                                             \
                    } while(0)

/**
 * \brief definitions sensor_data structs
 */
#endif
typedef uint16_t sensor_id_t;
typedef double sensor_value_t;

typedef time_t sensor_ts_t;         // UTC timestamp as returned by time() - notice that the size of time_t is different on 32/64 bit machine

/**
 * Struct used to store sensor data.*/
typedef struct {
    sensor_id_t id;
    sensor_value_t value;
    sensor_ts_t ts;
    bool is_datamgr;            /** a boolean detecting for which manager it is destined*/
} sensor_data_t;

/**
 * \brief definitions logger module
 */

#ifndef LOG_MESSAGE
/**
 * Use LOG_MESSAGE() te log messages to log file. It will put a the *formatted* message into the log file. together with a
 * timestamp as well a unique sequenced number.
 * \param ... formatted message form and variables for the message if any
 * */
#define LOG_MESSAGE(...) write_to_log_process(__VA_ARGS__)
#endif


#define BUFFER_SIZE 1000

#define LOG_FILE "gateway.log"


/**
 * \brief Write a FORMATTED message to the log process.
 * Uses ERROR_HANDLER() to deal with errors.
 * \param format formatted message to be written.
 * \param arguments for the format specifiers
 * \return EXIT_SUCCESS on success and exits with status code on errors
 */
int write_to_log_process(const char* format, ...);

/**
 * \brief Create the log process.
 * Uses ERROR_HANDLER to deal with errors.
 * \return EXIT_SUCCESS on success and exits with status code on errors
 */
int create_log_process();

/**
 * \brief End the log process.
 * Uses ERROR_HANDLER() to deal with errors.
 * \return EXIT_SUCCESS on success and exits with status code on errors
 */
int end_log_process();


#endif /* _CONFIG_H_ */
