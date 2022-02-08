#include "event_callback.h"

#include "yuv/libyuv.h"
#include <iostream>
#include "main_window.h"

EventCallback::EventCallback() {}

EventCallback::~EventCallback() {
    if (fout_.is_open()) {
        fout_.close();
    }
    running_ = false;
    cv_.notify_all();
    if (work_thread_.joinable()) {
        work_thread_.join();
    }
}

void EventCallback::OnScreenFrame(std::shared_ptr<VideoFrame> video_frame) {
    if (video_frame->GetFrameType() != kFrameTypeARGB) {
        return;
    }
    {
        std::unique_lock<std::mutex> lock(mtx_);
        frame_queue_.push(video_frame);
        cv_.notify_all();
    }
    if (running_) {
        return;
    }
    running_ = true;
    if (work_thread_.joinable()) {
        work_thread_.join();
    }
    work_thread_ = std::thread([&]() {
        while (running_) {
            std::shared_ptr<VideoFrame> frame;
            {
                std::unique_lock<std::mutex> lock(mtx_);
                if (frame_queue_.empty()) {
                    cv_.wait(lock);
                }
                if (!running_) {
                    return;
                }
                frame = frame_queue_.front();
                frame_queue_.pop();
            }
            int width = frame->GetWidth();
            int height = frame->GetHeight();
            if (!fout_ || !fout_.is_open()) {
                std::string filename = "../../../" +
                                       std::to_string(width) + std::to_string(height) + ".yuv";
                fout_.open(filename, std::ios::binary | std::ios::out);
            }
            if (width_ != width || height_ != height) {
                width_ = width;
                height_ = height;
                if (y_data_) {
                    delete[] y_data_;
                }
                if (u_data_) {
                    delete[] u_data_;
                }
                if (v_data_) {
                    delete[] v_data_;
                }
                y_data_ = new uint8_t[width_ * height_];
                u_data_ = new uint8_t[width_ * height_ / 4];
                v_data_ = new uint8_t[width_ * height_ / 4];
            }
            uint8_t* argb_data = frame->GetData();
            int ret = libyuv::ARGBToI420(argb_data, width * 4, y_data_, width, u_data_, width / 2,
                                         v_data_, width / 2, width, height);
            fout_.write((char*)y_data_, width * height);
            fout_.write((char*)u_data_, width * height / 4);
            fout_.write((char*)v_data_, width * height / 4);
        }
    });
}

void EventCallback::OnScreenEvent(ScreenEvent screen_event) {
    if (main_window_) {
        main_window_->OnScreenEvent(screen_event);
    }
}

void EventCallback::RegisteObserver(MainWindow* main_window) {
    main_window_ = main_window;
}