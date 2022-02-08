#pragma once

#include "video_common.h"
#include "video_event_handler.h"
#include "task_thread.h"
#include "video_capture_manager.h"

class VideoCaptureProxy {
public:
    VideoCaptureProxy();
    ~VideoCaptureProxy();

    void SetVideoProfile(const VideoProfile& video_profile);
    void StartCapture(const std::string& video_device_id);
    void StopCapture();
    void StartPreview(void* hwnd);
    void StopPreview();
    void RegisteVideoFrameObserver(std::shared_ptr<IVideoFrameObserver> observer);

private:
    TaskThread task_thread_{};
    VideoCaptureManager video_capture_manager_{};
};