#include "../include/client.h"
#include "../include/server.h"
#include <stdio.h>       // Include for fprintf() and stderr
#include <stdlib.h>      // For EXIT_SUCCESS, EXIT_FAILURE, strtol()
#include <sys/wait.h>    // For waitpid()
#include <unistd.h>      // For fork(), getpid(), sleep()

#define TEN 10

int main(int argc, const char *argv[])
{
    pid_t pid_server;

    // Check if the correct number of command-line arguments is provided
    if(argc != 3)
    {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Fork to create a separate process for the server
    pid_server = fork();

    if(pid_server < 0)
    {
        // Error occurred
        fprintf(stderr, "Fork for server failed\n");
        exit(EXIT_FAILURE);
    }
    else if(pid_server == 0)
    {
        // Child process: Run the server
        server(argv[1], (int)strtol(argv[2], NULL, TEN));
    }
    else
    {
        pid_t pid_client;

        // Parent process: Continue running the client
        printf("Server process (PID: %d) running...\n", pid_server);

        // Fork to create a separate process for the client
        pid_client = fork();

        if(pid_client < 0)
        {
            // Error occurred
            fprintf(stderr, "Fork for client failed\n");
            exit(EXIT_FAILURE);
        }
        else if(pid_client == 0)
        {
            // Child process: Run the client
            client(argc, argv);
        }
        else
        {
            // Parent process: Wait for both server and client to finish
            printf("Client process (PID: %d) running...\n", pid_client);
            // Wait for both child processes to finish
            waitpid(pid_server, NULL, 0);
            waitpid(pid_client, NULL, 0);
        }
    }

    // Return success
    return EXIT_SUCCESS;
}
