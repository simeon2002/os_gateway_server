//
// Created by sims0702 on 12/23/23.
//

#include "logger.h"

// GLOBAL VARS
int fd[2];
int i = 0;
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
            if (fprintf(fp, "%d - %s - %s\n", i++, timestamp, received_message) < 0) {
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
