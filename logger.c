//
// Created by sims0702 on 12/23/23.
//

#include "logger.h"
// only write_to_log_process will have a returning value.
//TODO: add error handler function like in plab1

// GLOBAL VARS
int fd[2];
int i = 0;
pthread_mutex_t logger_mutex;

/*global variables (for file opening + pipe)*/
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
        ERROR_HANDLER(1, MUTEX_ERROR, "Error during creation logger mutex");
    }
    // log file opening
    // creation of communication and child process
    if (pipe(fd) == -1) {
        pthread_mutex_destroy(&logger_mutex);
        ERROR_HANDLER(1, PIPE_CREATION_FAILURE, "Error occurred during pipe creation");
    };
    int pid = fork();
    // error handle
    if (pid == -1) {
        close(fd[0]);
        close(fd[1]);
        pthread_mutex_destroy(&logger_mutex);
        ERROR_HANDLER(1, FORK_FAILURE, "fork failed");
    } else if (pid == 0) { // logging process
        // opening log file.
        close(fd[1]); // closing write_end.
        FILE *fp = fopen(LOG_FILE, "w");
        if (fp == NULL) {
            close(fd[0]);
            pthread_mutex_destroy(&logger_mutex);
            ERROR_HANDLER(1, FILE_OPENING_ERROR, "Error occurred during log file opening");
        }
        char received_message[BUFFER_SIZE];
        size_t message_length;
        while(read(fd[0], &message_length, sizeof(size_t)) > 0) {
            if ((read(fd[0], received_message, message_length)) < 0) {
                fclose(fp);
                close(fd[0]);
                pthread_mutex_destroy(&logger_mutex);
                ERROR_HANDLER(1, PIPE_READING_ERROR, "An error occurred during reading from pipe");
            }
            printf("child process reads: %s \n", received_message);

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
                ERROR_HANDLER(1, LOG_WRITING_ERROR, "Error occurred during writing to log file");
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

int write_to_log_process(char *msg) {
    pthread_mutex_lock(&logger_mutex);
    size_t msg_length = strlen(msg) + 1;
    write(fd[1], &msg_length, sizeof(size_t)); // writing the length to be written.
    if (write(fd[1], msg, strlen(msg) + 1) == -1) {  // writing the message based on the length to be written. (includes \0)
        perror("Error occurred during writing to pipe");
        pthread_mutex_unlock(&logger_mutex);
        end_log_process();
        return PIPE_WRITING_ERROR; // TODO: Note the writing process WILL NOT BE terminated!!! very important to take into account!
    }
    pthread_mutex_unlock(&logger_mutex);
    return EXIT_SUCCESS;
}