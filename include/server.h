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
void  parse_and_execute_command(int client_socket, char *command);
int   find_binary_executable(const char *command, char *full_path);
void  execute_process(const char *full_path, char **args, int client_socket);

#endif /* SERVER_H_ */
