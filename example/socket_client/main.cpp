#include <iostream>
#include <thread>
#include <chrono>
#include "socket_client.h"

int main(void) {
    SocketClient socket_client;
	if (socket_client.ConnectServer("192.168.0.108", 32786)) {
		std::cout << "connect failed" << std::endl;
	}
    std::string message = "123";
    while (true) {
        socket_client.SendSocketMessage((uint8_t*)message.c_str(), message.length());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    socket_client.DisconnectServer();
    return 0;
}