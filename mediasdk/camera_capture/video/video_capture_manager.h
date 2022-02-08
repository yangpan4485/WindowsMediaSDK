#pragma once
#include <Windows.h>
#include <string>

#include "video_common.h"
#include "video_event_handler.h"
#include "video_capture.h"

class VideoCaptureManager {
public:
    VideoCaptureManager();
    ~VideoCaptureManager();

    void SetVideoProfile(const VideoProfile& video_profile);
    void StartCapture(const std::string& video_device_id);
    void StopCapture();
    void StartPreview(HWND hwnd);
    void StopPreview();
    void RegisteVideoFrameObserver(std::shared_ptr<IVideoFrameObserver> observer);

private:
    std::shared_ptr<VideoCapture> video_capture_{};
    VideoProfile video_profile_{};
};
