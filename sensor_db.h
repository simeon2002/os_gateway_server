/**
 * \author Simeon Serafimov
 */

#ifndef _SENSOR_DB_H_
#define _SENSOR_DB_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "config.h"
#include "logger.h"

/*error codes*/
// status 1: file opening error
#define FILE_OPENING_ERROR 1
// status 2: fork failure
#define FORK_FAILURE 2
// status 3: writing to db file failure
#define SENSOR_DB_WRITE_ERROR 3
// status 4: file failed to close
#define FILE_CLOSING_ERROR 4
// status 5: pipe creation failure
#define PIPE_CREATION_ERROR 5


/*storage manager functions*/
FILE * storagemgr_open_db(char * filename, bool append);
int storagemgr_insert_sensor(FILE * f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts);
int storagemgr_close_db(FILE * f, char *filename);


#endif /* _SENSOR_DB_H_ */