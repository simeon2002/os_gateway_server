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
        write_to_log_process("ERROR: could not open db file.");
        perror("Error occurred during file opening");
        exit(FILE_OPENING_ERROR);
    }

    // Communicate with the log process
    write_to_log_process("New DB file %s has been created.", filename);
    return fp;
}


int storagemgr_insert_sensor(FILE * f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
    if (fprintf(f, "%d, %lf, %ld \n", id, value, ts) < 0) {
        write_to_log_process("ERROR: could not write to DB file.");
        perror("Error writing to file");
        fclose(f);
        return SENSOR_DB_WRITE_ERROR;
    }
    fflush(f);
//    printf("this is the parent process during insertion of data \n");
    // Communicate with the log process
    write_to_log_process("Data insertion from sensor %d.", id);
    return 0;
}

int storagemgr_close_db(FILE * f, char *filename) {
    if ( fclose(f) == EOF) {
        write_to_log_process("ERROR: could not close DB file %s.", filename);
        perror("Error closing file");
        return FILE_CLOSING_ERROR;
    };
    // Communicate with the log process
    write_to_log_process("The %s DB file has been closed.", filename);
    return 0;
}