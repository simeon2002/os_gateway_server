//
// Created by sims0702 on 12/23/23.
//

#include "config.h"
#include "connmgr.h"
#include "sbuffer.h"
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


// LOGGER PROCESS FUNCTION DEFINITIONS
// GLOBAL VARS
int fd[2];
int logger_count = 0;
pthread_mutex_t logger_mutex;

int end_log_process() {
    close(fd[1]);
    int status;
    wait(&status);
    if (WIFEXITED(status)) { // checking the exit status of child to determine type of error via status code.
        int exit_status = WEXITSTATUS(status);
        if (exit_status != 0) {
            fprintf(stderr, "Child process exit with status %d", exit_status);
        }
    } else
        ERROR_HANDLER(1, EXIT_FAILURE, "Error occurred during wait for logging process");// --> indicates error occurrence during waiting time
    // or if child process is still running, i.e. it became a zombie process !!
    if (pthread_mutex_destroy(&logger_mutex) != 0) {
        perror("Mutex destruction failed");
    }
    return EXIT_SUCCESS;
}

int create_log_process() {
    // initializing logger mutex
    if (pthread_mutex_init(&logger_mutex, NULL) != 0) {
        ERROR_HANDLER(1, EXIT_MUTEX_ERROR, "Error during creation logger mutex");
    }

    // creation of communication and child process
    if (pipe(fd) == -1) {
        pthread_mutex_destroy(&logger_mutex);
        ERROR_HANDLER(1, EXIT_PIPE_ERROR, "Error occurred during pipe creation");
    };

    int pid = fork();
    // error handle
    if (pid == -1) {
        close(fd[0]);
        close(fd[1]);
        pthread_mutex_destroy(&logger_mutex);
        ERROR_HANDLER(1, EXIT_FORK_FAILURE, "fork failed");
    } else if (pid == 0) { // logging process
        // opening log file.
        close(fd[1]); // closing write_end.
        FILE *fp = fopen(LOG_FILE, "w");
        if (fp == NULL) {
            close(fd[0]);
            pthread_mutex_destroy(&logger_mutex);
            ERROR_HANDLER(1, EXIT_FILE_ERROR, "Error occurred during log file opening");
        }
        char received_message[BUFFER_SIZE];
        size_t message_length;
        while(read(fd[0], &message_length, sizeof(size_t)) > 0) {
            if ((read(fd[0], received_message, message_length)) < 0) {
                fclose(fp);
                close(fd[0]);
                pthread_mutex_destroy(&logger_mutex);
                ERROR_HANDLER(1, EXIT_PIPE_ERROR, "An error occurred during reading from pipe");
            }
//            printf("child process reads: %s \n", received_message);

            // timestamp creation.
            time_t timer;
            time(&timer);
            struct tm *timestamp_info;
            timestamp_info = localtime(&timer);
            char timestamp[25];
            strftime(timestamp, sizeof(timestamp), "%a %b %d %X %Y", timestamp_info);

            // writing message to log file.
            if (fprintf(fp, "%d - %s - %s\n", logger_count++, timestamp, received_message) < 0) {
                fclose(fp);
                close(fd[0]);
                pthread_mutex_destroy(&logger_mutex);
                ERROR_HANDLER(1, EXIT_LOG_ERROR, "Error occurred during writing to log file");
            }
            fflush(fp); // helps writing to log file without data to be buffered first.
        }
        fclose(fp);
        close(fd[0]);
        exit(EXIT_SUCCESS); // success status.
    } else {
        close(fd[0]);
    }
    return EXIT_SUCCESS;
}

int write_to_log_process(const char *format, ...) {
    pthread_mutex_lock(&logger_mutex);
    va_list args;
    va_start(args, format);

    // Determine the length of the formatted message
    size_t msg_length = vsnprintf(NULL, 0, format, args) + 1;

    // Allocate memory for the formatted message
    char *formatted_msg = (char*)malloc(msg_length);
    if (formatted_msg == NULL) {
        va_end(args);
        end_log_process();
        pthread_mutex_unlock(&logger_mutex);
        ERROR_HANDLER(1, EXIT_MEMORY_ALLOCATION_ERROR, "Error memory couldn't be allocated.");
    }

    // Reset the va_list to the beginning
    va_end(args);
    va_start(args, format);

    // Format the message using the va_list
    vsnprintf(formatted_msg, msg_length, format, args);

    // writing length to pipe.
    write(fd[1], &msg_length, sizeof(size_t));

    // writing formatted message to pipe.
    if (write(fd[1], formatted_msg, msg_length) == -1) {
        free(formatted_msg);
        end_log_process();
        va_end(args);
        pthread_mutex_unlock(&logger_mutex);
        ERROR_HANDLER(1, EXIT_PIPE_ERROR, "Error occurred during writing to pipe");
    }

    free(formatted_msg);
    va_end(args);
    pthread_mutex_unlock(&logger_mutex);
    return EXIT_SUCCESS;
}