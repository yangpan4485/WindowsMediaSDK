#pragma once
#include <Windows.h>
#include <GdiPlus.h>
#include <string>
#include <vector>

#include "screen/mag/screen_capture_mag.h"
#include "screen_common.h"

class ThumbnailManager {
public:
    ThumbnailManager();
    ~ThumbnailManager();

    void SetScreenList(std::vector<ScreenInfo> screen_list);
    std::string GetScreenThumbnail(WindowRect screen_rect, uint32_t thumbnail_width,
                                   uint32_t thumbnail_height);
    std::string GetWindowThumbnail(HWND window_id, uint32_t thumbnail_width,
                                   uint32_t thumbnail_height);

private:
    std::string GetThumbnailImage(Gdiplus::Bitmap&& bitmap_src, uint32_t thumbnail_width,
                                  uint32_t thumbnail_height);

private:
    ULONG_PTR token_{};
    std::vector<ScreenInfo> screen_list_{};
    int desktop_x_{};
    int desktop_y_{};
};