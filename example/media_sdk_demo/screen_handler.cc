#include "screen_handler.h"

#include "main_window.h"

ScreenCaptureHandler::ScreenCaptureHandler() {}

ScreenCaptureHandler::~ScreenCaptureHandler() {}

void ScreenCaptureHandler::OnScreenFrame(std::shared_ptr<VideoFrame> video_frame) {
    if (main_window_) {
        main_window_->OnScreenFrame(video_frame);
    }
}

void ScreenCaptureHandler::OnScreenEvent(ScreenEvent screen_event) {
    if (main_window_) {
        main_window_->OnScreenEvent(screen_event);
    }
}

void ScreenCaptureHandler::SetObserver(MainWindow* main_window) {
    main_window_ = main_window;
}