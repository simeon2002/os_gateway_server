//
// Created by sims0702 on 12/23/23.
//

#include "connection_mgr.h"
// global vars
int conn_counter = 0;

// client args
typedef struct client_args {
    tcpsock_t *client;
    int max_conn;
} client_args;

void *client_handler(client_args_t *args) {
    int bytes, result;
    tcpsock_t *client;
    client = args->client;
    sensor_data_t data;
    data.is_datamgr = false;

    do { // reading sensor data
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void *) &data.id, &bytes);
        bytes = sizeof(data.value);
        result = tcp_receive(client, (void *) &data.value, &bytes);
        bytes = sizeof(data.ts);
        result = tcp_receive(client, (void *) &data.ts, &bytes);
        if ((result == TCP_NO_ERROR) && bytes) {
            // todo: printf to be removed
            write_to_log_process("Sensor data received from peer");
            printf("\n\n writing:");
            printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value,
                    (long int) data.ts);
            fflush(stdout);
            data.is_datamgr = true;
//            sleep(1);
            sbuffer_insert(shared_buffer, &data, false);
            data.is_datamgr = false;
            sbuffer_insert(shared_buffer, &data, true);

        }

    } while (result == TCP_NO_ERROR);

    if (result == TCP_CONNECTION_CLOSED) {
        printf("Peer has closed connection\n");
        write_to_log_process("Peer has closed connection");
    }
    else
        printf("Error occurred on connection to peer\n");

    if (tcp_close(&client) != TCP_NO_ERROR) {
        fprintf(stderr, "Error closing client socket\n");
    }

    if (conn_counter == args->max_conn) { // only tell buffer to step at last connection.
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

        /*server socket creation*/
        tcpsock_t *server, *client;
        client_args_t *args = (client_args_t*)malloc(sizeof(client_args_t));
        if(argc < 3) {
            printf("Please provide the right arguments: first the port, then the max nb of clients");
            return -1;
        }
    int MAX_CONN = atoi(argv[2]); // use stroi
    int PORT = atoi(argv[1]);

        printf("Test server is started\n");
        if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) {
            ERROR_HANDLER(1, EXIT_TCP_ERROR, "Error during socket creation of server");
        }
    write_to_log_process("Server is started");

        /*thread creation for each client*/
        pthread_t thread_ids[MAX_CONN];

        do {
            if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) {
                ERROR_HANDLER(1, EXIT_TCP_ERROR, "error client listening: either, the port isn't correctly assigned, malloc error or socket operation error");
            }
            printf("Incoming client connection\n");
            write_to_log_process("Incoming client connection");
            args->max_conn = MAX_CONN;
            args->client = client;

            if (pthread_create(thread_ids + conn_counter, NULL, (void*)client_handler, args) != 0) {
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
         write_to_log_process("Shutting down of server");

        return 0;
    }