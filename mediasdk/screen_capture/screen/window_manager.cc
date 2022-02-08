#include "window_manager.h"

#include <iostream>

#include "window_utils.h"

WindowManager::WindowManager() {
    // CoInitialize(NULL);
    // CoInitializeEx(NULL, COINIT_MULTITHREADED);
}

WindowManager::~WindowManager() {
    CoUninitialize();
}

std::vector<WindowInfo> WindowManager::GetWindowList() {
    std::vector<WindowInfo> window_list;
    EnumWindows(EnumWindowProc, (LPARAM)&window_list);
    for (auto it = window_list.begin(); it != window_list.end();) {
        RECT rect;
        GetWindowRect(it->hwnd, &rect);
        it->window_rect.left = rect.left;
        it->window_rect.right = rect.right;
        it->window_rect.top = rect.top;
        it->window_rect.bottom = rect.bottom;
        if (!IsWindowVisibleOnCurrentDesktop(it->hwnd)) {
            it = window_list.erase(it);
        }
        else {
            ++it;
        }
    }
    return window_list;
}

std::vector<ScreenInfo> WindowManager::GetScreenList() {
    std::vector<HMONITOR> monitors;
    screen_list_.clear();
    EnumMonitor(monitors);
    for (size_t i = 0; i < monitors.size(); ++i) {
        MONITORINFOEX mie;
        memset(&mie, 0, sizeof(MONITORINFOEX));
        mie.cbSize = sizeof(MONITORINFOEX);
        if (!GetMonitorInfo(monitors[i], &mie)) {
            continue;
        }
        ScreenInfo screen_info;
        screen_info.monitor = monitors[i];
        screen_info.display_id = i;
        screen_info.is_primary = ((mie.dwFlags & MONITORINFOF_PRIMARY) != 0) ? true : false;
        screen_info.screen_rect = GetScreenRect(i);
        screen_list_.push_back(std::move(screen_info));
        /*
        // 受 dpi 影响
        window_info.rect.left = mie.rcMonitor.left;
        window_info.rect.top = mie.rcMonitor.top;
        window_info.rect.right = mie.rcMonitor.right;
        window_info.rect.bottom = mie.rcMonitor.bottom;
        */
    }
    thumbnail_manager_.SetScreenList(screen_list_);
    return screen_list_;
}

std::string WindowManager::GetWindowThumbImage(HWND hwnd, uint32_t width, uint32_t height) {
    if (screen_list_.empty()) {
        GetScreenList();
        thumbnail_manager_.SetScreenList(screen_list_);
    }
    return thumbnail_manager_.GetWindowThumbnail(hwnd, width, height);
}

std::string WindowManager::GetScreenThumbImage(ScreenInfo window_info, uint32_t width, uint32_t height) {
    return thumbnail_manager_.GetScreenThumbnail(window_info.screen_rect, width, height);
}