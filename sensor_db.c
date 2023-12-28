#include "sensor_db.h"

FILE* storagemgr_open_db(char *filename, bool append) {

    FILE *fp;
    if (append == true) {
        fp = fopen(filename, "a");
    } else {
        fp = fopen(filename, "w");
    }
    // Error handling for file opening
    if (fp == NULL) {
        ERROR_HANDLER(1, EXIT_FILE_ERROR, "Error: could not open db file.");
    }

    // Communicate with the log process
    LOG_MESSAGE("New DB file %s has been created.", filename);
    return fp;
}


int storagemgr_insert_sensor(FILE * f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
    if (fprintf(f, "%d, %lf, %ld \n", id, value, ts) < 0) {
        fclose(f);
        ERROR_HANDLER(1, EXIT_FILE_ERROR, "Error could not write to DB file.");
    }
    fflush(f);
    // Communicate with the log process
    LOG_MESSAGE("Data insertion from sensor %d.", id);
    return EXIT_SUCCESS;
}

int storagemgr_close_db(FILE * f, char *filename) {
    if ( fclose(f) == EOF) {
        ERROR_HANDLER(1, EXIT_FILE_ERROR, "Error: could not close DB file.");
    };
    // Communicate with the log process
    LOG_MESSAGE("The %s DB file has been closed.", filename);
    return EXIT_SUCCESS;
}