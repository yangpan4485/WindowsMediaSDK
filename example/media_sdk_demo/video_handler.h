#pragma once

#include "video_event_handler.h"

class MainWindow;
class VideoFrameObserver : public IVideoFrameObserver, public IVideoDeviceEventHandler {
public:
    VideoFrameObserver();
    ~VideoFrameObserver();
    void OnVideoError(int error_code, const std::string& device_name) override;

    void OnVideoFrame(std::shared_ptr<VideoFrame> video_frame) override;

    void SetObserver(MainWindow* main_window);

    void OnVideoDeviceEvent(VideoDeviceEvent device_event, const std::string& device_id) override;

private:
    MainWindow* main_window_{};
};
