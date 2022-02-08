#pragma once
#include "video_event_handler.h"
#include "video_frame.h"
class MainWindow;
class VideoEventHandler : public IVideoDeviceEventHandler {
public:
    VideoEventHandler();
    ~VideoEventHandler();
    void OnVideoDeviceEvent(VideoDeviceEvent device_event, const std::string& device_name) override;
};

class VideoFrameObserver : public IVideoFrameObserver {
public:
    VideoFrameObserver();
    ~VideoFrameObserver();

    void OnVideoError(int error_code, const std::string& device_name) override;
    void OnVideoFrame(std::shared_ptr<VideoFrame> video_frame) override;
    void SetObserver(MainWindow* main_window);

private:
    MainWindow* main_window_{};
};