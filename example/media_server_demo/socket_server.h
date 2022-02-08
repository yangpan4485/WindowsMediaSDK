#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <fstream>
#include "video_render.h"
#include "video_decoder.h"
class SocketServer {
public:
    SocketServer();
    ~SocketServer();

    void Startup();
    void Shutdown();
	void SetWindow(HWND hwnd);

private:
    std::thread work_thread_{};
    bool running_ = false;
	std::shared_ptr<VideoRender> video_render_{};
	std::shared_ptr<VideoDecoder> video_decoder_{};
    WSADATA wsa_data_;
    SOCKET listen_socket_{INVALID_SOCKET};
    SOCKET client_socket_{INVALID_SOCKET};
	HWND render_window_{};

	uint8_t* y_data_{};
	uint8_t* u_data_{};
	uint8_t* v_data_{};
	uint32_t frame_width_{};
	uint32_t frame_height_{};
};