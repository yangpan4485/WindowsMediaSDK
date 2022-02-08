#pragma once
#include <memory>
#include <vector>

#include "video_common.h"
#include "video_event_handler.h"

class VideoCaptureEngine {
public:
    VideoCaptureEngine();
    ~VideoCaptureEngine();

    void SetVideoProfile(const VideoProfile& video_profile);
    void StartCapture(const std::string& video_device_id);
    void StopCapture();
    void StartPreview(void* hwnd);
    void StopPreview();
    void RegisteVideoFrameObserver(std::shared_ptr<IVideoFrameObserver> observer);
    std::string GetCurrentDevice();

    void RegisteVideoDeviceEventHandler(std::shared_ptr<IVideoDeviceEventHandler> handler);
    std::vector<VideoDeviceInfo> EnumVideoDevices();

private:
    class VideoCaptureEngineImpl;
    std::unique_ptr<VideoCaptureEngineImpl> pimpl_{};
};