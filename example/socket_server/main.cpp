#include <iostream>
#include "socket_server.h"

int main(void) {
    SocketServer server;
    server.Startup();
    getchar();
    return 0;
}