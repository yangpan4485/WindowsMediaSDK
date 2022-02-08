#pragma once
#include <Windows.h>

class TransparentWindow {
public:
    TransparentWindow(int display_id);
    TransparentWindow(HWND window_id);
    ~TransparentWindow();
    HWND GetTransparentWindowId();
    bool DrawTransWinow();

private:
    bool CreateTransWindow();
    bool DestroyTransWindow();
    bool DrawTransWinowBorder();
    RECT GetCaptureSourceRect();
    static LRESULT CALLBACK WndProc(HWND hWND, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    bool is_display_{};
    bool is_window_{};
    bool need_update_{ true };
    int display_id_{};
    HWND window_id_{};
    HWND transport_window_{};
    RECT draw_rect_{};
};
