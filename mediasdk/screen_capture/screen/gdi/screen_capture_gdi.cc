#include "screen_capture_gdi.h"

#include <cstdint>
#include <dwmapi.h>
#include <iostream>

#include "common/version_helper.h"

ScreenCaptureGDI::ScreenCaptureGDI() {}

ScreenCaptureGDI::~ScreenCaptureGDI() {
    Uninit();
}

std::shared_ptr<VideoFrame> ScreenCaptureGDI::CaptureWindow(const WindowInfo& window_info) {
    is_display_ = false;
    if (capture_hwnd_ != window_info.hwnd) {
        init_ = false;
        capture_hwnd_ = window_info.hwnd;
    }
    if (!init_) {
        Init();
    }
    RECT rect;
    ::GetWindowRect(capture_hwnd_, &rect);
    return CaptureFrame(rect);
}

std::shared_ptr<VideoFrame> ScreenCaptureGDI::CaptureScreen(const ScreenInfo& screen_info) {
    is_display_ = true;
    // 先不考虑分辨率变化的情况
    if (!init_) {
        Init();
    }
    RECT rect;
    rect.left = screen_info.screen_rect.left;
    rect.right = screen_info.screen_rect.right;
    rect.top = screen_info.screen_rect.top;
    rect.bottom = screen_info.screen_rect.bottom;
    return CaptureFrame(rect);
}

std::shared_ptr<VideoFrame> ScreenCaptureGDI::CaptureFrame(const RECT& rect) {
    uint32_t width = rect.right - rect.left;
    uint32_t height = rect.bottom - rect.top;
    if (width_ != width || height_ != height) {
        width_ = width;
        height_ = height;
        return nullptr;
    }
    // ScreenImage
    auto image = std::make_shared<BitmapGDI>(memory_dc_, width, height, bitmap_pool_);
    
    if (old_bmp_ == nullptr) {
        old_bmp_ = SelectObject(memory_dc_, image->GetBitmap());
    } else {
        SelectObject(memory_dc_, image->GetBitmap());
    }
    BOOL result = false;
    if (!is_display_) {
        if (VersionHelper::Instance().IsWindows10OrLater()) {
            const UINT flags = PW_RENDERFULLCONTENT;;
            result = PrintWindow(capture_hwnd_, memory_dc_, flags);
            if (!result) {
                std::cout << "capture failed" << std::endl;
            }
        }
        if (!result) {
            result = PrintWindow(capture_hwnd_, memory_dc_, 0);
        }
    }
    if (!result) {
        DWORD rop = SRCCOPY;
        BOOL bDwmEnabled = TRUE;
        HRESULT hr = DwmIsCompositionEnabled(&bDwmEnabled);
        if (hr == S_OK && !bDwmEnabled) {
            rop |= CAPTUREBLT;
        }
        result = BitBlt(memory_dc_, 0, 0, width, height, desktop_dc_, rect.left, rect.top, SRCCOPY | CAPTUREBLT);
        if (result == FALSE) {
            std::cout << "BitBlt false" << std::endl;
            return false;
        }
    }
    if (enable_cursor_) {
        // draw Cursor
        CURSORINFO cursorInfo;
        memset(&cursorInfo, 0, sizeof(cursorInfo));
        cursorInfo.cbSize = sizeof(cursorInfo);
        if (::GetCursorInfo(&cursorInfo) && (cursorInfo.flags & CURSOR_SHOWING)) {
            if (::PtInRect(&rect, cursorInfo.ptScreenPos)) {
                ::DrawIcon(memory_dc_, cursorInfo.ptScreenPos.x - rect.left,
                           cursorInfo.ptScreenPos.y - rect.top, cursorInfo.hCursor);
            }
        }
    }
    std::shared_ptr<VideoFrame> video_frame = nullptr;
    if (is_display_) {
        video_frame.reset(new VideoFrame(width, height, kFrameTypeARGB, true));
    }
    else {
        video_frame.reset(new VideoFrame(width, height, kFrameTypeARGB, false));
    }
    memcpy(video_frame->GetData(), image->GetARGBData(), image->GetSize());
    return video_frame;
}

bool ScreenCaptureGDI::Init() {
    Uninit();
    DwmEnableComposition(DWM_EC_DISABLECOMPOSITION);
    if (is_display_) {
        desktop_dc_ = GetDC(NULL);
    } else {
        desktop_dc_ = GetDC(capture_hwnd_);
    }
    memory_dc_ = CreateCompatibleDC(desktop_dc_);
    return true;
}

bool ScreenCaptureGDI::Uninit() {
    if (old_bmp_) {
        SelectObject(memory_dc_, old_bmp_);
        old_bmp_ = nullptr;
    }
    if (desktop_dc_) {
        ReleaseDC(NULL, desktop_dc_);
        desktop_dc_ = nullptr;
    }
    if (memory_dc_) {
        DeleteDC(memory_dc_);
        memory_dc_ = nullptr;
    }
    return true;
}