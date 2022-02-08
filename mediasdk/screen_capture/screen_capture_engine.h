#pragma once
#include <vector>
#include <string>
#include <memory>

#include "screen_common.h"
#include "screen_event_handler.h"

class ScreenCaptureEngine {
public:
    ScreenCaptureEngine();
    ~ScreenCaptureEngine();

    std::vector<ScreenInfo> GetScreenLists();
    std::vector<WindowInfo> GetWindowLists();

    std::string GetWindowThumbImage(const WindowInfo& window_info, uint32_t thumb_width, uint32_t thumb_height);
    std::string GetScreenThumbImage(const ScreenInfo& screen_info, uint32_t thumb_width, uint32_t thumb_height);

    void StartCaptureDisplay(const ScreenInfo& screen_info, const CaptureConfig& config, const std::vector<HWND>& ignore_window);
    void StartCaptureWindow(const WindowInfo& window_info, const CaptureConfig& config);
    void StopCapture();
    void RegisterCaptureHandler(std::shared_ptr<ICaptureHandler> handler);

private:
    class ScreenCaptureEngineImpl;
    std::shared_ptr<ScreenCaptureEngineImpl> pimpl_{};
};