#pragma once
#include <Windows.h>
#include <functional>
#include <thread>

#include "video_common.h"
#include "video_event_handler.h"
using VideoDeviceChangeCallback =
    std::function<void(VideoDeviceEvent video_device_event, const std::string& device_id)>;

class VideoDeviceMonitor {
public:
    VideoDeviceMonitor();
    ~VideoDeviceMonitor();
    void SetVideoDeviceChangeCallback(VideoDeviceChangeCallback callback);

private:
    bool Start();
    bool Stop();
    bool Init();
    UINT JudgeDevice(WPARAM wParam, LPARAM lParam, VideoDeviceEvent event);

private:
    friend LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    VideoDeviceChangeCallback callback_{};
    std::thread work_{};
    bool running_{false};
    HWND hwnd_;
    std::string last_device_id_{};
    VideoDeviceEvent last_event_{};
};