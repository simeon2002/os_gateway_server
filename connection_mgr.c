//
// Created by sims0702 on 12/23/23.
//

#include "connection_mgr.h"


// global vars
int conn_counter = 0;
// other conn_counter needs to be used otherwise a valid sensor id will be seen as invalid whenever multiple sensors are
// used with different sensor times because then conn_couter would have become max_conn even though the last client_thread
// hasn't finished yet.
int conn_counter_client_thread = 0;
pthread_mutex_t client_counter_mutex;

int MAX_CONN;

void *client_handler(tcpsock_t *client) {
    int bytes, result;
    sensor_data_t data;
    data.is_datamgr = false;
    int timeout_flag = 0;
    int invalid_sensor_flag = 0;
    int counter = 0; // used to display opening connection log msg.
    do { // reading sensor data
        time_t ts_receive_start = time(NULL);
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void *) &data.id, &bytes);
        if (counter == 0) { // log message for new opened connection
            write_to_log_process("Sensor node %d has opened a new connection", data.id);
            counter++;
        }
        bytes = sizeof(data.value);
        result = tcp_receive(client, (void *) &data.value, &bytes);
        bytes = sizeof(data.ts);
        result = tcp_receive(client, (void *) &data.ts, &bytes);

        // timeout break
        time_t ts_receive_end = time(NULL);
        if (ts_receive_end - ts_receive_start > TIMEOUT){
            timeout_flag = 1;
        }

        if (counter == 2 && datamgr_is_invalid_sensor(data.id)) {// after 5 counts, invalid sensor will be closed.
            invalid_sensor_flag = 1;
        }

        if ((result == TCP_NO_ERROR) && bytes && !timeout_flag) {
            write_to_log_process("Sensor data received from sensor node %d", data.id);
            fflush(stdout);
            data.is_datamgr = true;
            sbuffer_insert(shared_buffer, &data, false);
            data.is_datamgr = false;
            sbuffer_insert(shared_buffer, &data, true);
            counter++;
        }
    } while (result == TCP_NO_ERROR && !timeout_flag && !invalid_sensor_flag);
    if (result == TCP_CONNECTION_CLOSED) {
        printf("Peer has closed connection\n");
        write_to_log_process("Sensor node %d has closed the connection", data.id);
    }
    else if (timeout_flag) {
        printf("Timeout occurred, closing connection.\n");
        write_to_log_process("Timeout occurred, closing connection with sensor node %d.", data.id);
    }  else if (invalid_sensor_flag) {
        write_to_log_process("Invalid sensor id, closing connection with sensor node %d,", data.id);
    }

    else {
        printf("Error occurred on connection to peer\n");
    }

    if (tcp_close(&client) != TCP_NO_ERROR) {
        fprintf(stderr, "Error closing client socket\n");
    }

    pthread_mutex_lock(&client_counter_mutex);
    conn_counter_client_thread++;
    pthread_mutex_unlock(&client_counter_mutex);
    if (conn_counter_client_thread == MAX_CONN) { // only tell buffer to step at last connection.
        data.id = 0;
        data.is_datamgr = false;
        sbuffer_insert(shared_buffer, &data, false);
    //    sleep(1);
        data.is_datamgr = true;
        sbuffer_insert(shared_buffer, &data, true);
        sleep(1);
    }
    return NULL;
}

int cmgr_start_server(int argc, char *argv[]) {
        // main process creating server socket and threads for each client.

        // initializing mutex for client counter
    pthread_mutex_init(&client_counter_mutex, NULL);
        /*server socket creation*/
        tcpsock_t *server, *client;
        if(argc < 3) {
            printf("Please provide the right arguments: first the port, then the max nb of clients");
            return -1;
        }
    MAX_CONN = atoi(argv[2]); // use stroi
    int PORT = atoi(argv[1]);

        printf("Test server is started\n");
        if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) {
            ERROR_HANDLER(1, EXIT_TCP_ERROR, "Error during socket creation of server");
        }
    write_to_log_process("---SERVER STARTUP---");

        /*thread creation for each client*/
        pthread_t thread_ids[MAX_CONN];

    do {
            if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) {
                ERROR_HANDLER(1, EXIT_TCP_ERROR, "error client listening: either, the port isn't correctly assigned, malloc error or socket operation error");
            }
            printf("Incoming client connection\n");
            write_to_log_process("---INCOMING CLIENT CONNECTION---");

            if (pthread_create(thread_ids + conn_counter, NULL, (void*)client_handler, client) != 0) {
                ERROR_HANDLER(1, EXIT_THREAD_ERROR, "Error during thread creation for client.");
            }
            conn_counter++;

        } while (conn_counter < MAX_CONN);

        for (int i = 0; i < MAX_CONN; i++) {
            if (pthread_join(thread_ids[i], NULL) != 0) {
                ERROR_HANDLER(1, EXIT_THREAD_ERROR, "Error during thread join.");
            }
        }
        if (tcp_close(&server) != TCP_NO_ERROR) {
            ERROR_HANDLER(1, EXIT_TCP_ERROR, "Error during closing TCP connection.");
        }
        printf("Test server is shutting down\n");
         write_to_log_process("---SERVER SHUTDOWN---");

        return 0;
    }