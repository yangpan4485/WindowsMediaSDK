#pragma once

#include <Windows.h>
#include "bitmap_gdi.h"
#include "screen/screen_capture.h"
#include "video_frame.h"

struct ScreenCaptureGDIParam {
    bool enable_bitblt;
    bool show_cursor;
};

class ScreenCaptureGDI: public ScreenCapture {
public:
    ScreenCaptureGDI();
    ~ScreenCaptureGDI();

    std::shared_ptr<VideoFrame> CaptureWindow(const WindowInfo& window_info) override;
    std::shared_ptr<VideoFrame> CaptureScreen(const ScreenInfo& screen_info) override;

private:
    bool Init();
    bool Uninit();
    std::shared_ptr<VideoFrame> CaptureFrame(const RECT& rect);

private:
    HWND capture_hwnd_{};
    bool is_display_{};

    HDC desktop_dc_{};
    HDC memory_dc_{};
    int bitmap_width_{};
    int bitmap_height_{};
    HBITMAP bitmap_{};

    BitmapImageBufferPoolType bitmap_pool_;
    HGDIOBJ old_bmp_ = nullptr;
    bool init_ = false;
    uint32_t width_{};
    uint32_t height_{};
};