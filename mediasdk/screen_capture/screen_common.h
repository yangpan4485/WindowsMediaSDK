#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>

enum ScreenEvent {
    kScreenEventRemove = 0,
};

struct WindowRect {
    int left;
    int right;
    int top;
    int bottom;
};

struct WindowInfo {
    HWND hwnd;
    std::string window_title;
    uint32_t window_width;
    uint32_t window_height;
    WindowRect window_rect;
};

struct ScreenInfo {
    HMONITOR monitor;
    uint32_t display_id;
    uint32_t screen_width;
    uint32_t screen_height;
    WindowRect screen_rect;
    bool is_primary;
};

struct CaptureConfig {
    uint32_t frame_rate{};
    bool enable_border{};
    bool hd_mode_{};
};