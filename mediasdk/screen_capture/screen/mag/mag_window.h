#pragma once

#include <Windows.h>

struct MagWindowParam {
    int x{};
    int y{};
    int width{};
    int height{};
    void* param{};
};

class MagWindow {
public:
    MagWindow();
    ~MagWindow();
    bool CreateMagWindow(MagWindowParam window_param);
    void DestroyMagWindow();
    HWND GetHostWindow();
    HWND GetControlWindow();
    static bool RegisteWindowProc(WNDPROC proc);

private:
    HWND control_window_{};
    HWND host_window_{};
    static bool is_registed_;
};
