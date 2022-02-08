#include "mag_window.h"
#include <magnification.h>

#include <iostream>

bool MagWindow::is_registed_ = false;
constexpr TCHAR* kHostName = TEXT("MagHost");
constexpr TCHAR* kControlName = TEXT("MagControl");

MagWindow::MagWindow() {}

MagWindow::~MagWindow() {
    DestroyMagWindow();
}

bool MagWindow::CreateMagWindow(MagWindowParam window_param) {
    host_window_ =
        CreateWindowEx(0, kHostName, kHostName, WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                       window_param.x, window_param.y, window_param.width, window_param.height,
                       NULL, NULL, NULL, window_param.param);
    SetWindowLong(host_window_, GWL_EXSTYLE,
                  ::GetWindowLong(host_window_, GWL_EXSTYLE) | WS_EX_LAYERED);
    control_window_ =
        ::CreateWindowEx(0, WC_MAGNIFIER, kControlName, WS_CHILD | MS_SHOWMAGNIFIEDCURSOR, 0, 0,
                         window_param.width, window_param.height, host_window_, NULL, NULL, NULL);
    std::cout << "width: " << window_param.width << std::endl;
    std::cout << "height: " << window_param.height << std::endl;
    if (control_window_ == NULL) {
        DestroyWindow(host_window_);
        host_window_ = NULL;
        return false;
    }
    return true;
}

void MagWindow::DestroyMagWindow() {
    if (host_window_) {
        DestroyWindow(host_window_);
        host_window_ = nullptr;
    }
    if (control_window_) {
        DestroyWindow(control_window_);
        control_window_ = nullptr;
    }
}

HWND MagWindow::GetHostWindow() {
    return host_window_;
}

HWND MagWindow::GetControlWindow() {
    return control_window_;
}

bool MagWindow::RegisteWindowProc(WNDPROC proc) {
    if (!is_registed_) {
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = proc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = NULL;
        wcex.hIcon = NULL;
        wcex.hCursor = NULL;
        wcex.hbrBackground = NULL;
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = TEXT("MagHost");
        wcex.hIconSm = NULL;
        if (RegisterClassEx(&wcex)) {
            is_registed_ = true;
        }
    }
    return is_registed_;
}
