#include "video_capture_engine.h"

#include <algorithm>
#include <iostream>

#include "local_log.h"
#include "video/video_capture.h"
#include "video/video_capture_proxy.h"
#include "video/video_device_manager.h"
#include "video_log.h"

class VideoCaptureEngine::VideoCaptureEngineImpl {
public:
    VideoCaptureEngineImpl() {
        video_device_manager_.SetVideoDeviceChangeCallback(
            [this](VideoDeviceEvent video_device_event, const std::string& device_id) {
                // 每次插入和删除需要更新 video_devices_
                LOGI(kCameraLogTag)
                    << "OnVideoDeviceChange video_device_event: " << video_device_event
                    << ", device_id: " << device_id;
                std::string change_device_id = device_id;
                video_devices_ = video_device_manager_.EnumVideoDevices();
                if (video_device_event == kVideoDeviceRemove) {
                    auto pos = device_id.find("{");
                    std::string id = device_id.substr(0, pos);
                    std::transform(id.begin(), id.end(), id.begin(), ::tolower);
                    if (current_device_id_.find(id) != std::string::npos) {
                        change_device_id = current_device_id_;
                        StopCapture();
                        if (!video_devices_.empty()) {
                            StartCapture(video_devices_[0].device_id);
                        }
                    }
                }
                auto handler = video_device_event_handler_.lock();
                if (handler) {
                    handler->OnVideoDeviceEvent(video_device_event, change_device_id);
                }
            });
    }

    ~VideoCaptureEngineImpl() {}

    void SetVideoProfile(const VideoProfile& video_profile) {
        LOGI(kCameraLogTag) << "SetVideoProfile width: " << video_profile.width
                            << ", height: " << video_profile.height
                            << ", fps: " << video_profile.fps;
        video_capture_proxy_.SetVideoProfile(video_profile);
    }

    void StartCapture(const std::string& video_device_id) {
        LOGI(kCameraLogTag) << "StartCapture video_device_id: " << video_device_id;
        if (video_device_id.empty()) {
            return;
        }
        current_device_id_ = video_device_id;
        video_capture_proxy_.StartCapture(video_device_id);
    }

    void StopCapture() {
        LOGI(kCameraLogTag) << "StopCapture";
        video_capture_proxy_.StopCapture();
    }

    void StartPreview(void* hwnd) {
        video_capture_proxy_.StartPreview(hwnd);
    }

    void StopPreview() {
        video_capture_proxy_.StopPreview();
    }

    void RegisteVideoDeviceEventHandler(std::shared_ptr<IVideoDeviceEventHandler> handler) {
        video_device_event_handler_ = handler;
    }

    void RegisteVideoFrameObserver(std::shared_ptr<IVideoFrameObserver> observer) {
        video_capture_proxy_.RegisteVideoFrameObserver(observer);
    }

    std::vector<VideoDeviceInfo> EnumVideoDevices() {
        if (video_devices_.empty()) {
            video_devices_ = video_device_manager_.EnumVideoDevices();
        }
        return video_devices_;
    }

    std::string GetCurrentDevice() {
        return current_device_id_;
    }

private:
    VideoDeviceManager video_device_manager_;
    VideoCaptureProxy video_capture_proxy_;
    std::weak_ptr<IVideoDeviceEventHandler> video_device_event_handler_{};
    std::string current_device_id_{};
    std::vector<VideoDeviceInfo> video_devices_{};
};

VideoCaptureEngine::VideoCaptureEngine() : pimpl_(new VideoCaptureEngineImpl()) {}

VideoCaptureEngine::~VideoCaptureEngine() {}

void VideoCaptureEngine::SetVideoProfile(const VideoProfile& video_profile) {
    pimpl_->SetVideoProfile(video_profile);
}

void VideoCaptureEngine::StartCapture(const std::string& video_device_id) {
    pimpl_->StartCapture(video_device_id);
}

void VideoCaptureEngine::StopCapture() {
    pimpl_->StopCapture();
}

void VideoCaptureEngine::StartPreview(void* hwnd) {
    pimpl_->StartPreview(hwnd);
}

void VideoCaptureEngine::StopPreview() {
    pimpl_->StopPreview();
}

void VideoCaptureEngine::RegisteVideoDeviceEventHandler(
    std::shared_ptr<IVideoDeviceEventHandler> handler) {
    pimpl_->RegisteVideoDeviceEventHandler(handler);
}

void VideoCaptureEngine::RegisteVideoFrameObserver(std::shared_ptr<IVideoFrameObserver> observer) {
    pimpl_->RegisteVideoFrameObserver(observer);
}

std::string VideoCaptureEngine::GetCurrentDevice() {
    return pimpl_->GetCurrentDevice();
}

std::vector<VideoDeviceInfo> VideoCaptureEngine::EnumVideoDevices() {
    return pimpl_->EnumVideoDevices();
}