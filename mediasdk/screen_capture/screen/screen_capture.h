#pragma once
#include "screen_common.h"
#include "screen_event_handler.h"
#include <thread>
#include <vector>

#include "transparent_window.h"

class ScreenCapture {
public:
    ScreenCapture();
    virtual ~ScreenCapture();
    virtual void StartCaptureDisplay(const ScreenInfo& screen_info, const CaptureConfig& config);
    virtual void StartCaptureWindow(const WindowInfo& window_info, const CaptureConfig& config);
    virtual void StopCapture();
    virtual void RegisterCaptureHandler(std::shared_ptr<ICaptureHandler> handler);
    virtual void SetIgnoreWindowList(const std::vector<HWND>& ignore_window_list);
    virtual std::shared_ptr<VideoFrame> CaptureWindow(const WindowInfo& window_info);
    virtual std::shared_ptr<VideoFrame> CaptureScreen(const ScreenInfo& screen_info);

protected:
    void OnFrame(std::shared_ptr<VideoFrame> video_frame);

private:
    void ScreenCaptureLoop();

protected:
    bool enable_cursor_{true};
    ScreenInfo screen_info_;
    WindowInfo window_info_;
    std::vector<HWND> ignore_window_list_{};
    std::shared_ptr<TransparentWindow> transparent_window_{};
    bool capture_screen_{};

private:
    bool running_{};
    
    std::thread capture_thread_{};
    std::weak_ptr<ICaptureHandler> capture_handler_{};
    CaptureConfig capture_config_{};
    
};