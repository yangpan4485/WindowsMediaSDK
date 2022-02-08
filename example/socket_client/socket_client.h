#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <cstdint>

class SocketClient {
public:
    SocketClient();
    ~SocketClient();

    bool ConnectServer(const std::string& ip, uint16_t port);
    bool SendSocketMessage(uint8_t* data, uint32_t size);
    bool DisconnectServer();

private:
    SOCKET socket_client_{INVALID_SOCKET};
};