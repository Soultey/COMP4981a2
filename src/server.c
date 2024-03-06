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
#define MESSAGE "Message received"    // Hardcoded message

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
        ssize_t bytes_sent;

        bytes_received = read(client_socket, buffer, BUFFER_SIZE - 1);

        if(bytes_received <= 0)
        {
            printf("Client disconnected\n");
            close(client_socket);
            return NULL;
        }

        bytes_sent = send(client_socket, MESSAGE, strlen(MESSAGE), 0);
        if(bytes_sent == -1)
        {
            perror("Send failed");
            close(client_socket);
            exit(EXIT_FAILURE);
        }

        buffer[bytes_received] = '\0';

        printf("Received from client: %s\n", buffer);

        parse_and_handle_command(buffer);

        // Optional: Add code here to process the received message

        // Clear the buffer for receiving the next message
        memset(buffer, 0, sizeof(buffer));
    }

    return NULL;
}

void parse_and_handle_command(const char *buffer)
{
    // Parse the buffer and handle the command
    // You can implement your logic here to interpret the command
    printf("your mum: %s\n", buffer);
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
