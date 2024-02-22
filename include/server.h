// server.h

#ifndef SERVER_H_
#define SERVER_H_

#include <pthread.h>

#define BUFFER_SIZE 1024
#define TEN 10
#define FIVE 5

// Function declarations
void  server(const char *server_ip, int server_port) __attribute__((noreturn));
int   create_server_socket(const char *server_ip, int server_port);
void *client_handler(void *args);

#endif /* SERVER_H_ */
