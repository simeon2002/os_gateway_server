//
// Created by sims0702 on 12/23/23.
//

#include "connection_mgr.h"


void *client_handler(tcpsock_t *client) {
    int bytes, result;
    sensor_data_t data;

    do { // reading sensor data
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void *) &data.id, &bytes);
        bytes = sizeof(data.value);
        result = tcp_receive(client, (void *) &data.value, &bytes);
        bytes = sizeof(data.ts);
        result = tcp_receive(client, (void *) &data.ts, &bytes);
        if ((result == TCP_NO_ERROR) && bytes) {
            printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value,
                    (long int) data.ts);
        }

    } while (result == TCP_NO_ERROR);

    if (result == TCP_CONNECTION_CLOSED)
        printf("Peer has closed connection\n");
    else
        printf("Error occurred on connection to peer\n");

    if (tcp_close(&client) != TCP_NO_ERROR) {
        fprintf(stderr, "Error closing client socket\n");
    }
    return 0;
}

int cmgr_start_server(int argc, char *argv[]) {
        // main process creating server socket and threads for each client.

        /*server socket creation*/
        tcpsock_t *server, *client;
        int conn_counter = 0;

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

        /*thread creation for each client*/
        pthread_t thread_ids[MAX_CONN];

        do {
            if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) {
                ERROR_HANDLER(1, EXIT_TCP_ERROR, "error client listening: either, the port isn't correctly assigned, malloc error or socket operation error");
            }
            printf("Incoming client connection\n");

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
        return 0;
    }