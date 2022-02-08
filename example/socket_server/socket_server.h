#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>

class SocketServer {
public:
    SocketServer();
    ~SocketServer();

    void Startup();
    void Shutdown();

private:
    std::thread work_thread_{};
    bool running_ = true;
    WSADATA wsa_data_;
    SOCKET listen_socket_{INVALID_SOCKET};
    SOCKET client_socket_{INVALID_SOCKET};
};