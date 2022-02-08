#include "socket_server.h"
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

SocketServer::SocketServer() {
    WSAStartup(MAKEWORD(2, 2), &wsa_data_);
}

SocketServer::~SocketServer() {
    WSACleanup();
}

void SocketServer::Startup() {
    listen_socket_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	int ret = 0;
    sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = PF_INET;
    sockAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    sockAddr.sin_port = htons(32786);
	struct timeval timeout = { 6,0 };
	ret = setsockopt(listen_socket_, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
	std::cout << "ret: " << ret << std::endl;
    ret = bind(listen_socket_, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
    if (ret == SOCKET_ERROR) {
        closesocket(listen_socket_);
        return;
    }
	
    ret = listen(listen_socket_, 5);
    SOCKADDR client_addr;
    int size = sizeof(SOCKADDR);
    std::cout << "accept start" << std::endl;
    client_socket_ = ::accept(listen_socket_, (SOCKADDR*)&client_addr, &size);
	if (client_socket_ == INVALID_SOCKET) {
		std::cout << "accept failed" << std::endl;
	}
    std::cout << "accept end" << std::endl;
    char* recvbuf = new char[1024 * 102];
#if 1
    while (running_) {
        ret = recv(client_socket_, recvbuf, 1024 * 1024, 0);
        if (ret > 0) {
            std::cout << "recv ret: " << ret << std::endl;
        }
        else {
            break;
        }
    }
#endif
    delete[] recvbuf;
}

void SocketServer::Shutdown() {
    if (client_socket_ != INVALID_SOCKET) {
        shutdown(client_socket_, SD_SEND);
        closesocket(client_socket_);
    }
    if (listen_socket_) {
        closesocket(listen_socket_);
    }
}