#include "video_capture_manager.h"

#include <iostream>

#include "video_capture_factory.h"
#include "video_device_monitor.h"
#include "video_event_handler.h"

VideoCaptureManager::VideoCaptureManager() {
    video_profile_.width = 1280;
    video_profile_.height = 720;
    video_profile_.fps = 15;
}

VideoCaptureManager::~VideoCaptureManager() {}

void VideoCaptureManager::SetVideoProfile(const VideoProfile& video_profile) {
    video_profile_ = video_profile;
}

void VideoCaptureManager::StartCapture(const std::string& video_device_id) {
    if (!video_capture_) {
        video_capture_ = VideoCaptureFactory::GetInstance().CreateVideoCapture();
    }
    // 不同的 capture 自己计算需要的 format
    video_capture_->SetVideoProfile(video_profile_);
    video_capture_->StartCapture(video_device_id);
}

void VideoCaptureManager::StopCapture() {
    if (!video_capture_) {
        return;
    }
    video_capture_->StopCapture();
}

void VideoCaptureManager::StartPreview(HWND hwnd) {
    if (!video_capture_) {
        video_capture_ = VideoCaptureFactory::GetInstance().CreateVideoCapture();
    }
}

void VideoCaptureManager::StopPreview() {
    if (!video_capture_) {
        return;
    }
}

void VideoCaptureManager::RegisteVideoFrameObserver(std::shared_ptr<IVideoFrameObserver> observer) {
    if (!video_capture_) {
        video_capture_ = VideoCaptureFactory::GetInstance().CreateVideoCapture();
    }
    video_capture_->RegisteVideoFrameObserver(observer);
}