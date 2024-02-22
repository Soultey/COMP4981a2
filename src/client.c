// client.c

#include "../include/client.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef SOCK_CLOEXEC
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-macros"
    #define SOCK_CLOEXEC 0
    #pragma GCC diagnostic pop
#endif

#define BUFFER_SIZE 1024
#define TEN 10

int main(int argc, const char *argv[])
{
    char              *server_ip;
    long               server_port;
    struct sockaddr_in server_address;
    int                client_socket;
    char               message[BUFFER_SIZE];
    char               buffer[BUFFER_SIZE];
    char              *endptr;

    if(argc != 3)
    {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Allocate memory for server_ip and copy argv[1] into it
    server_ip = strdup(argv[1]);
    if(server_ip == NULL)
    {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Convert string to long integer using strtol for better error handling
    server_port = strtol(argv[2], &endptr, TEN);
    if(*endptr != '\0')
    {
        fprintf(stderr, "Invalid server port: %s\n", argv[2]);
        free(server_ip);    // Free memory allocated for server_ip before exiting
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port   = htons((uint16_t)server_port);

    client_socket = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if(client_socket == -1)
    {
        perror("Socket creation failed");
        free(server_ip);    // Free memory allocated for server_ip before exiting
        exit(EXIT_FAILURE);
    }

    if(inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0)
    {
        perror("Invalid address / Address not supported");
        free(server_ip);    // Free memory allocated for server_ip before exiting
        exit(EXIT_FAILURE);
    }

    if(connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Connection failed");
        free(server_ip);    // Free memory allocated for server_ip before exiting
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        ssize_t bytes_sent;

        ssize_t bytes_received;
        // Get message from client's terminal
        printf("Enter message to send to server: ");
        fgets(message, BUFFER_SIZE, stdin);
        // Remove newline character if present
        message[strcspn(message, "\n")] = '\0';

        // Check if the user
        // Check if the user wants to exit
        if(strcmp(message, "exit") == 0)
        {
            // Send an exit signal to the server
            send(client_socket, message, strlen(message), 0);
            break;    // Exit the loop
        }

        // Send the message to the server
        bytes_sent = send(client_socket, message, strlen(message), 0);
        if(bytes_sent == -1)
        {
            perror("Send failed");
            free(server_ip);    // Free memory allocated for server_ip before exiting
            exit(EXIT_FAILURE);
        }

        // Receive response from the server
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if(bytes_received == -1)
        {
            perror("Receive failed");
            free(server_ip);    // Free memory allocated for server_ip before exiting
            exit(EXIT_FAILURE);
        }

        // Null-terminate the received data
        buffer[bytes_received] = '\0';

        printf("Received from server: %s\n", buffer);
    }

    // Close the socket
    close(client_socket);

    // Free memory allocated for server_ip
    free(server_ip);

    return 0;
}
