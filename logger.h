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
#include <stdarg.h>
#include "config.h"

#define BUFFER_SIZE 1000
#define LOG_FILE "gateway.log"

#ifndef LOG_MESSAGE
#define LOG_MESSAGE(...) write_to_log_process(__VA_ARGS__)
#endif


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

#endif //OS_PROJECT_LOGGER_H
