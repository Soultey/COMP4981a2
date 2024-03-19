#include "../include/server.h"
#include <arpa/inet.h>
#include <pthread.h>    // Include pthread library for multi-threading
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define TEN 10
#define FIVE 5
#define FULL_PATH_LENGTH 256
#define ERROR_MESSAGE_SIZE 19

#ifndef SOCK_CLOEXEC
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-macros"
    #define SOCK_CLOEXEC 0
    #pragma GCC diagnostic pop
#endif

void server(const char *server_ip, int server_port)
{
    int server_socket = create_server_socket(server_ip, server_port);
    printf("Server listening on port %d...\n", server_port);

    while(1)
    {
        struct sockaddr_in client_address;
        socklen_t          client_address_len;
        int                client_socket;
        pthread_t          thread_id;

        client_address_len = sizeof(client_address);
        client_socket      = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if(client_socket == -1)
        {
            perror("Accept failed");
            continue;
        }
        printf("Client connected\n");

        // Create a new thread to handle the client
        if(pthread_create(&thread_id, NULL, client_handler, (void *)&client_socket) != 0)
        {
            perror("Thread creation failed");
            close(client_socket);
            continue;
        }
        // Detach the thread to avoid resource leak
        pthread_detach(thread_id);
    }
}

int create_server_socket(const char *server_ip, int server_port)
{
    int                server_socket;
    struct sockaddr_in server_address;

    server_socket = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if(server_socket == -1)
    {
        perror("Server socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family      = AF_INET;
    server_address.sin_port        = htons((uint16_t)server_port);
    server_address.sin_addr.s_addr = inet_addr(server_ip);

    if(bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if(listen(server_socket, FIVE) == -1)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    return server_socket;
}

void *client_handler(void *args)
{
    int  client_socket = *((int *)args);
    char buffer[BUFFER_SIZE];

    while(1)
    {
        ssize_t bytes_received;

        bytes_received = read(client_socket, buffer, BUFFER_SIZE - 1);

        if(bytes_received <= 0)
        {
            printf("Client disconnected\n");
            close(client_socket);
            return NULL;
        }

        buffer[bytes_received] = '\0';

        printf("Received from client: %s\n", buffer);

        parse_and_execute_command(client_socket, buffer);

        // Optional: Add code here to process the received message

        // Clear the buffer for receiving the next message
        memset(buffer, 0, sizeof(buffer));
    }

    return NULL;
}

int find_binary_executable(const char *command, char *full_path)
{
    char      *savePtr     = NULL;    // Reset savePtr to NULL before each call
    const char delimiter[] = ":";
    char      *path        = getenv("PATH");
    char      *path_token;

    if(path != NULL)
    {
        printf("PATH environment variable: %s\n", path);
    }
    else
    {
        printf("PATH environment variable is not set\n");
    }

    if(path == NULL)
    {
        fprintf(stderr, "PATH environment variable not found.\n");
        return EXIT_FAILURE;
    }

    printf("Attempting to find %s in directories:\n", command);

    // Reset savePtr to NULL before each call to strtok_r
    savePtr = NULL;

    path_token = strtok_r(path, delimiter, &savePtr);

    path = NULL;

    while(path_token != NULL)
    {
        snprintf(full_path, FULL_PATH_LENGTH, "%s/%s", path_token, command);
        // Checks if the path is an executable file

        if(access(full_path, X_OK) == 0)
        {
            // Binary executable found
            return EXIT_SUCCESS;
        }
        path_token = strtok_r(NULL, delimiter, &savePtr);
    }
    fprintf(stderr, "Command %s was not found.\n", command);

    return EXIT_FAILURE;
}

void parse_and_execute_command(int client_socket, char *command)
{
    char  full_path[FULL_PATH_LENGTH];
    char *args[BUFFER_SIZE];    // Buffer to hold command and arguments
    int   argc = 0;
    char *token;
    char *savePtr;    // For strtok_r

    // Tokenize the command and arguments
    token = strtok_r(command, " ", &savePtr);
    while(token != NULL && argc < BUFFER_SIZE - 1)
    {
        args[argc++] = token;
        token        = strtok_r(NULL, " ", &savePtr);
    }
    args[argc] = NULL;    // Ensure the last argument is NULL

    // Find the binary executable path for the command
    if(find_binary_executable(args[0], full_path) == EXIT_FAILURE)
    {
        char error_message[ERROR_MESSAGE_SIZE];
        strcpy(error_message, "Command not found\n");
        send(client_socket, error_message, strlen(error_message), 0);
        return;
    }

    // Execute the command process
    execute_process(full_path, args, client_socket);

    savePtr = NULL;
}

void execute_process(const char *full_path, char **args, int client_socket)
{
    pid_t pid = fork();
    if(pid == -1)
    {
        perror("Error creating child process");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)
    {
        // Child process
        // Redirect stdout to client socket
        if(dup2(client_socket, STDOUT_FILENO) == -1)
        {
            perror("Error redirecting stdout");
            exit(EXIT_FAILURE);
        }
        // Close client socket descriptor inherited from parent
        close(client_socket);

        // Execute the command
        execv(full_path, args);

        // If execv fails, print error and exit child process
        perror("Error executing command");
        exit(EXIT_FAILURE);
    }
    else
    {
        // Parent process
        int status;
        wait(&status);
        if(WIFEXITED(status))
        {
            printf("Child process exited with status: %d\n", WEXITSTATUS(status));
        }
    }
}

int main(int argc, const char *argv[])
{
    long        port;
    const char *server_ip;

    if(argc != 3)
    {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    server_ip = argv[1];
    port      = strtol(argv[2], NULL, TEN);

    server(server_ip, (int)port);
}
