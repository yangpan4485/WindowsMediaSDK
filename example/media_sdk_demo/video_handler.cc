#include "video_handler.h"

#include <iostream>

#include "main_window.h"

VideoFrameObserver::VideoFrameObserver() {}

VideoFrameObserver::~VideoFrameObserver() {}

void VideoFrameObserver::OnVideoError(int error_code, const std::string& device_name) {}

void VideoFrameObserver::OnVideoFrame(std::shared_ptr<VideoFrame> video_frame) {
    if (main_window_) {
        main_window_->OnVideoFrame(video_frame);
    }
}

void VideoFrameObserver::SetObserver(MainWindow* main_window) {
    main_window_ = main_window;
}

void VideoFrameObserver::OnVideoDeviceEvent(VideoDeviceEvent device_event,
                                            const std::string& device_id) {
    if (main_window_) {
        main_window_->OnVideoDeviceEvent(device_event, device_id);
    }
}