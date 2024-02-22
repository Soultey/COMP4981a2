#include "../include/server.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define TEN 10
#define FIVE 5

#ifndef SOCK_CLOEXEC
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-macros"
    #define SOCK_CLOEXEC 0
    #pragma GCC diagnostic pop
#endif

// Modify the server function to pass the client socket as an argument
void server(const char *server_ip, int server_port)
{
    int server_socket = create_server_socket(server_ip, server_port);
    printf("Server listening on port %d...\n", server_port);

    while(1)
    {
        struct sockaddr_in client_address;
        socklen_t          client_address_len;
        int                client_socket;

        client_address_len = sizeof(client_address);
        client_socket      = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if(client_socket == -1)
        {
            perror("Accept failed");
            continue;
        }
        printf("Client connected\n");
    }
}

// Create and bind server socket
int create_server_socket(const char *server_ip, int server_port)
{
    int                server_socket;
    struct sockaddr_in server_address;

    // Create a socket for the server
    server_socket = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if(server_socket == -1)
    {
        perror("Server socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server_address struct
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family      = AF_INET;
    server_address.sin_port        = htons((uint16_t)server_port);
    server_address.sin_addr.s_addr = inet_addr(server_ip);

    // Bind the server socket to the server_address
    if(bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if(listen(server_socket, FIVE) == -1)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    return server_socket;
}

// Client handler thread function
void *client_handler(void *args)
{
    int  client_socket = *((int *)args);    // Extract the client socket from the argument
    char buffer[BUFFER_SIZE];

    while(1)
    {
        ssize_t bytes_received = read(client_socket, buffer, sizeof(buffer) - 1);

        if(bytes_received <= 0)
        {
            // Handle read error or closed socket
            printf("Client disconnected\n");
            close(client_socket);
            return NULL;
        }

        // Null-terminate the received data
        buffer[bytes_received] = '\0';

        // Print the received information
        printf("Received from client: %s\n", buffer);
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

/**
 * TODO:
 * Concurrency
 * Allow messages to be independently handled per thread
 * Currently the server can't handle more than 1 message per client at a time
 * Currently the server will send a message received by 1 client to another automatically
 * Use the same commands as A2 T3
 *
 */
