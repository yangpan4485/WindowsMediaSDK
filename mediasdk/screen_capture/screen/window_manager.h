#pragma once
#include <vector>

#include "screen_common.h"
#include "thumbnail/thumbnail_manager.h"

class WindowManager {
public:
    WindowManager();
    ~WindowManager();

    std::vector<WindowInfo> GetWindowList();
    std::vector<ScreenInfo> GetScreenList();

    std::string GetWindowThumbImage(HWND hwnd, uint32_t width, uint32_t height);
    std::string GetScreenThumbImage(ScreenInfo window_info, uint32_t width, uint32_t height);

private:
    ThumbnailManager thumbnail_manager_;
    std::vector<ScreenInfo> screen_list_{};
};
