// server.h

#ifndef SERVER_H_
#define SERVER_H_

#include <pthread.h>

#define BUFFER_SIZE 1024
#define TEN 10
#define FIVE 5
#define MAX_CLIENTS 10


struct ThreadArgs
{
    int client_socket;
};

// Function declarations
void  server(const char *server_ip, int server_port);
int   create_server_socket(const char *server_ip, int server_port);
void *client_handler(void *args);

#endif /* SERVER_H_ */
