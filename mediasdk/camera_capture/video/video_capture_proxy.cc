#include "video_capture_proxy.h"

VideoCaptureProxy::VideoCaptureProxy() {}

VideoCaptureProxy::~VideoCaptureProxy() {
    task_thread_.Wait();
}

void VideoCaptureProxy::SetVideoProfile(const VideoProfile& video_profile) {
    task_thread_.PostWork([&]() { video_capture_manager_.SetVideoProfile(video_profile); });
}

void VideoCaptureProxy::StartCapture(const std::string& video_device_id) {
    task_thread_.PostWork([&]() { video_capture_manager_.StartCapture(video_device_id); });
}

void VideoCaptureProxy::StopCapture() {
    task_thread_.PostWork([&]() { video_capture_manager_.StopCapture(); });
}

void VideoCaptureProxy::StartPreview(void* hwnd) {
    task_thread_.PostWork([&]() { video_capture_manager_.StartPreview((HWND)hwnd); });
}

void VideoCaptureProxy::StopPreview() {
    task_thread_.PostWork([&]() { video_capture_manager_.StopPreview(); });
}

void VideoCaptureProxy::RegisteVideoFrameObserver(std::shared_ptr<IVideoFrameObserver> observer) {
    video_capture_manager_.RegisteVideoFrameObserver(observer);
}