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

/*storage manager functions*/

/**
 * This method open the database to be written to.
 * Use of ERROR_HANLDER() if error occurs.
 * \param filename nameo of DB file to open
 * \param append file mode to be opened, append if true and write if false
 * \return FILE* file pointer to the opened file
 * */
FILE * storagemgr_open_db(char * filename, bool append);

/**
 * This method insert sensor data into the DB file.
 * Use of ERROR_HANDLER() if error occurs.
 * \param f file pointer to DB file
 * \param id sensor id of sensor data
 * \param ts timestamp of sensor data
 * \param value temperature value of sensor data
 * \return returns EXIT_SUCCESS on success or exits with appropriate status code on error.
 * */
int storagemgr_insert_sensor(FILE * f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts);

/**
 * This method closes the DB file.
 * Use of ERROR_HANDLER() if error occurs.
 * \param f file pointer to DB file
 * \param filename filename of the DB file
 * \return returns EXIT_SUCCESS on success or exits with appropriate status code on error.
 * */
int storagemgr_close_db(FILE * f, char *filename);


#endif /* _SENSOR_DB_H_ */