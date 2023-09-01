#include "screen_capture.h"

#include <chrono>
#include <iostream>
#include <functional>

ScreenCapture::ScreenCapture() {}

ScreenCapture::~ScreenCapture() {
    StopCapture();
}

void ScreenCapture::StartCaptureDisplay(const ScreenInfo& screen_info,
                                        const CaptureConfig& config) {
    if (running_) {
        return;
    }
    running_ = true;
    screen_info_ = screen_info;
    capture_screen_ = true;
    capture_config_ = config;
    capture_thread_ = std::thread(std::bind(&ScreenCapture::ScreenCaptureLoop, this));
}

void ScreenCapture::StartCaptureWindow(const WindowInfo& window_info, const CaptureConfig& config) {
    if (running_) {
        return;
    }
    if (::IsIconic(window_info.hwnd)) {
        BOOL result = ::ShowWindow(window_info.hwnd, SW_RESTORE);
        if (!result) {
            result = SendMessage(window_info.hwnd, WM_SYSCOMMAND, SC_RESTORE, NULL);
        }
    }
    if (window_info.hwnd) {
        DWORD process_thread_id = GetWindowThreadProcessId(GetForegroundWindow(), LPDWORD(0));
        DWORD current_thread_id = GetCurrentThreadId();
        AttachThreadInput(process_thread_id, current_thread_id, true);
        BringWindowToTop(window_info.hwnd);
        AttachThreadInput(process_thread_id, current_thread_id, false);
    }
    running_ = true;
    capture_config_ = config;
    window_info_ = window_info;
    capture_screen_ = false;
    capture_thread_ = std::thread(std::bind(&ScreenCapture::ScreenCaptureLoop, this));
}

void ScreenCapture::StopCapture() {
    if (!running_) {
        return;
    }
    running_ = false;
    if (capture_thread_.joinable()) {
        capture_thread_.join();
    }
}

void ScreenCapture::RegisterCaptureHandler(std::shared_ptr<ICaptureHandler> handler) {
    capture_handler_ = handler;
}

void ScreenCapture::OnFrame(std::shared_ptr<VideoFrame> video_frame) {
    auto handler = capture_handler_.lock();
    if (!handler) {
        return;
    }
    handler->OnScreenFrame(video_frame);
}

void ScreenCapture::ScreenCaptureLoop() {
    while (running_) {
#if 1
        if (!transparent_window_) {
            if (capture_screen_) {
                // screen capture 绿框标识窗口会很卡
                // transparent_window_.reset(new TransparentWindow(screen_info_.display_id));
            } else {
                transparent_window_.reset(new TransparentWindow(window_info_.hwnd));
            }
            // ignore_window_list_.push_back(transparent_window_->GetTransparentWindowId());
        }
        if (transparent_window_) {
            MSG message;
            if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
        }
        std::shared_ptr<VideoFrame> video_frame;
        if (capture_screen_) {
            video_frame = CaptureScreen(screen_info_);
        } else {
            // 需要判断窗口是不是存在的
            if (!::IsWindow(window_info_.hwnd)) {
                auto handler = capture_handler_.lock();
                if (handler) {
                    handler->OnScreenEvent(kScreenEventRemove);
                }
                break;
            }
            video_frame = CaptureWindow(window_info_);
        }
        uint32_t sleep_time = 100;
        if (video_frame) {
            auto handler = capture_handler_.lock();
            if (handler) {
                handler->OnScreenFrame(video_frame);
            }
        }
        else {
            std::cout << "video_frame is nullptr" << std::endl;
            sleep_time = sleep_time * 2 > 1000 ? 1000 : sleep_time * 2;
        }
        // transparent_window_->DrawTransWinow();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
#endif
    }
}

void ScreenCapture::SetIgnoreWindowList(const std::vector<HWND>& ignore_window_list) {
    ignore_window_list_ = ignore_window_list;
}

std::shared_ptr<VideoFrame> ScreenCapture::CaptureWindow(const WindowInfo& window_info) {
    return nullptr;
}

std::shared_ptr<VideoFrame> ScreenCapture::CaptureScreen(const ScreenInfo& screen_info) {
    return nullptr;
}