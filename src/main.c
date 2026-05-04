#include "game.h"
#include "server.h"
#include <stdlib.h>

int main(int argc, char **argv)
{
    /* Use the default port unless the user passed a custom one. */
    int port = DEFAULT_PORT;

    if (argc > 1) {
        port = atoi(argv[1]);
    }

    return start_server(port);
}
