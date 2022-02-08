#pragma once

#include <Windows.h>
#include <thread>

#include <dwmapi.h>

class ThumbnailWindow {
public:
    ThumbnailWindow();
    ~ThumbnailWindow();

    bool Create(const RECT& rect);
    bool Destroy();
    bool SetTarget(HWND hWndTarget);

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    RECT window_rect_{};
    bool create_result_{};
    HANDLE create_event_{};
    std::thread create_thread_{};
    HWND thumbanil_hwnd_{};
    HTHUMBNAIL thumbnail_id_{};
};