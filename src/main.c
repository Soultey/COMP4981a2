#include "../include/client.h"
#include "../include/server.h"
#include <stdlib.h>

int main(void)
{
    client("Hello, World");
    server("Hello, World");

    return EXIT_SUCCESS;
}
