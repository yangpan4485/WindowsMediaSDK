#include "video_device_event_handler.h"

#include <iostream>

#include "main_window.h"

VideoEventHandler::VideoEventHandler() {}

VideoEventHandler::~VideoEventHandler() {}

void VideoEventHandler::OnVideoDeviceEvent(VideoDeviceEvent device_event,
                                           const std::string& device_name) {}

VideoFrameObserver::VideoFrameObserver() {}

VideoFrameObserver::~VideoFrameObserver() {}

void VideoFrameObserver::OnVideoError(int error_code, const std::string& device_name) {}

void VideoFrameObserver::OnVideoFrame(std::shared_ptr<VideoFrame> video_frame) {
    if (main_window_) {
        main_window_->OnFrame(video_frame);
    }
}

void VideoFrameObserver::SetObserver(MainWindow* main_window) {
    main_window_ = main_window;
}