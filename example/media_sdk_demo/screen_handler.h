#pragma once

#include "screen_event_handler.h"

class MainWindow;
class ScreenCaptureHandler : public ICaptureHandler {
public:
    ScreenCaptureHandler();
    ~ScreenCaptureHandler();

    void OnScreenFrame(std::shared_ptr<VideoFrame> video_frame) override;
    void OnScreenEvent(ScreenEvent screen_event) override;

    void SetObserver(MainWindow* main_window);

private:
    MainWindow* main_window_{};
};
