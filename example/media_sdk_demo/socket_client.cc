#include "socket_client.h"

#include <iostream>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

uint8_t send_buffer[1024 * 1024];

SocketClient::SocketClient() {
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2, 2), &wsa_data);
}

SocketClient::~SocketClient() {
	WSACleanup();
}

bool SocketClient::ConnectServer(const std::string& ip, uint16_t port) {
	socket_client_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = PF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(ip.c_str());
	sockAddr.sin_port = htons(port);
	struct timeval timeout = {6,0};
	setsockopt(socket_client_, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval));
	int ret = connect(socket_client_, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
	std::cout << "connect result: " << ret << std::endl;
	if (ret == 0) {
		return true;
	}
	return false;
}

bool SocketClient::SendSocketMessage(uint8_t* data, uint32_t size) {
    *(uint32_t*)send_buffer = htonl(size);
    memcpy(send_buffer + 4, data, size);
    int ret = send(socket_client_, (const char*)send_buffer, size + 4, 0);
	if (ret == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

bool SocketClient::DisconnectServer() {
	if (socket_client_ != INVALID_SOCKET) {
		int ret = shutdown(socket_client_, SD_SEND);
		if (ret == SOCKET_ERROR) {
			return false;
		}
		closesocket(socket_client_);
	}
	return true;
}