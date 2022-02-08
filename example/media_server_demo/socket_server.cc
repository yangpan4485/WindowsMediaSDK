#include "socket_server.h"
#include "libyuv.h"
#include "video_decoder_factory.h"
#include "video_render_factory.h"
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")

char recvbuf[1024 * 1024];

SocketServer::SocketServer() {
    video_render_ = VideoRenderFactory::CreateInstance()->CreateVideoRender(kRenderTypeOpenGL);
    video_decoder_ = VideoDecoderFactory::GetInstance().CreateVideoDecoder();
    video_decoder_->SetDevoceFrameCallback([&](const std::shared_ptr<VideoFrame>& video_frame) {
        if (frame_width_ != video_frame->GetWidth() || frame_height_ != video_frame->GetHeight()) {
            frame_width_ = video_frame->GetWidth();
            frame_height_ = video_frame->GetHeight();
            std::cout << "frame_width_: " << frame_width_ << std::endl;
            std::cout << "frame_height_: " << frame_height_ << std::endl;
            if (y_data_) {
                delete[] y_data_;
            }
            if (u_data_) {
                delete[] u_data_;
            }
            if (v_data_) {
                delete[] v_data_;
            }
            y_data_ = new uint8_t[frame_width_ * frame_height_];
            u_data_ = new uint8_t[frame_width_ * frame_height_ / 4];
            v_data_ = new uint8_t[frame_width_ * frame_height_ / 4];
        }
        if (video_frame->GetFrameType() == kFrameTypeI420) {
            // 可以不拷贝，直接渲染
            memcpy(y_data_, video_frame->GetData(), frame_width_ * frame_height_);
            memcpy(u_data_, video_frame->GetData() + frame_width_ * frame_height_,
                   frame_width_ * frame_height_ / 4);
            memcpy(v_data_, video_frame->GetData() + frame_width_ * frame_height_ * 5 / 4,
                   frame_width_ * frame_height_ / 4);
        } else {
            libyuv::NV12ToI420(video_frame->GetData(), frame_width_,
                               video_frame->GetData() + frame_width_ * frame_height_, frame_width_,
                               y_data_, frame_width_, u_data_, frame_width_ / 2, u_data_,
                               frame_width_ / 2, frame_width_, frame_height_);
        }

        video_render_->RendFrameI420(y_data_, frame_width_, u_data_, frame_width_ / 2, v_data_,
                                     frame_width_ / 2, frame_width_, frame_height_);
    });
    WSAStartup(MAKEWORD(2, 2), &wsa_data_);
    listen_socket_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = PF_INET;
    sockAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    sockAddr.sin_port = htons(32786);
    int ret = bind(listen_socket_, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
    if (ret == SOCKET_ERROR) {
        closesocket(listen_socket_);
        return;
    }
    ret = listen(listen_socket_, 5);
}

SocketServer::~SocketServer() {
    Shutdown();
    WSACleanup();
}

void SocketServer::Startup() {
    if (running_) {
        return;
    }
    running_ = true;
    work_thread_ = std::thread([&]() {
        SOCKADDR client_addr;
        int size = sizeof(SOCKADDR);
        std::cout << "accept start" << std::endl;
        client_socket_ = accept(listen_socket_, (SOCKADDR*)&client_addr, &size);
        std::cout << "accept end" << std::endl;
        fd_set read_set;
        struct timeval timeout;
        FD_ZERO(&read_set);
        int max_fd = client_socket_ + 1;
        uint32_t h264_len_buf;
#if 1
        while (running_) {
            FD_SET(client_socket_, &read_set);
            timeout.tv_sec = 60;
            timeout.tv_usec = 0;
            int n = select(max_fd, &read_set, NULL, NULL, &timeout);
            if (n > 0) {
                int read_len =
                    recv(client_socket_, (char*)&h264_len_buf, sizeof(h264_len_buf), MSG_WAITALL);
                if (read_len < sizeof(h264_len_buf)) {
                    printf("recv packet length error");
                    break;
                }
                int h264_len = ntohl(h264_len_buf);
                read_len = recv(client_socket_, recvbuf, h264_len, MSG_WAITALL);

                if (read_len < h264_len) {
                    printf("recv h264 data  error");
                    break;
                }
                video_decoder_->Decode((uint8_t*)recvbuf, h264_len);
            } else {
                printf("select error %d\n", errno);
            }
        }
#endif
    });
}

void SocketServer::Shutdown() {
    running_ = false;
    if (work_thread_.joinable()) {
        work_thread_.detach();
    }
    if (client_socket_ != INVALID_SOCKET) {
        shutdown(client_socket_, SD_SEND);
        closesocket(client_socket_);
    }
    if (listen_socket_) {
        closesocket(listen_socket_);
    }
}

void SocketServer::SetWindow(HWND hwnd) {
    video_render_->SetWindow(hwnd);
}