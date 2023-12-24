//
// Created by sims0702 on 12/23/23.
//

#ifndef OS_PROJECT_LOGGER_H
#define OS_PROJECT_LOGGER_H
/**
 * \author BL
 * \file logger.h
 * \brief Header file for logger module
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define BUFFER_SIZE 1000
#define LOG_FILE "gateway.log"

// status codes
#define FILE_OPENING_ERROR 1
#define FORK_FAILURE 2
#define LOG_WRITING_ERROR 3
#define FILE_CLOSING_ERROR 4
#define PIPE_CREATION_FAILURE 5
#define PIPE_READING_ERROR 6
#define PIPE_WRITING_ERROR 7
#define MUTEX_ERROR 8

/*
 * Use ERROR_HANDLER() for handling memory allocation problems, invalid sensor IDs, non-existing files, etc.
 */
#ifndef ERROR_HANDLER
#define ERROR_HANDLER(condition, exit_status, ...)    do {                       \
                      if (condition) {                              \
                        printf("\nError: in %s - function %s at line %d: %s\n", __FILE__, __func__, __LINE__, __VA_ARGS__); \
                        exit(exit_status);                         \
                      }                                             \
                    } while(0)
#endif
/**
 * \brief Write a message to the log process.
 * \param msg The message to be written.
 * \return Status code.
 */
int write_to_log_process(char *msg);

/**
 * \brief Create the log process.
 * \return Status code.
 */
int create_log_process();

/**
 * \brief End the log process.
 * \return Status code.
 */
int end_log_process();

#endif //OS_PROJECT_LOGGER_H
