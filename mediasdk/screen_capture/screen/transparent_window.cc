#include "transparent_window.h"

#include <cstdint>
#include <dwmapi.h>
#include <GdiPlus.h>
#include <iostream>

#include "version_helper.h"
#include "window_utils.h"


const char* kClassName = "TransparentWindow";

const int32_t kWindowAeroWidth = 9;
const int32_t kBorderWidth = 4;
const uint8_t kRedColor = 0;
const uint8_t kGreenColor = 250;
const uint8_t kBlueColor = 0;
const uint8_t kAlpha = 200;

LRESULT CALLBACK TransparentWindow::WndProc(HWND hWND, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_ERASEBKGND:
        return 0;
    case WM_CLOSE:
        DestroyWindow(hWND);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWND, msg, wParam, lParam);
    }
    return 0;
}

TransparentWindow::TransparentWindow(int display_id) : display_id_(display_id), is_display_(true) {
    CreateTransWindow();
}

TransparentWindow::TransparentWindow(HWND window_id) : window_id_(window_id), is_window_(true) {
    CreateTransWindow();
}

TransparentWindow::~TransparentWindow() {
    DestroyTransWindow();
}

HWND TransparentWindow::GetTransparentWindowId() {
    return transport_window_;
}

bool TransparentWindow::CreateTransWindow() {
    HINSTANCE hinstance = GetModuleHandle(0);
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hinstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = kClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        return false;
    }
    transport_window_ = CreateWindowEx(
        WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        kClassName, kClassName, WS_POPUP | WS_VISIBLE, 0, 0, 0, 0, NULL, NULL, hinstance, NULL);
    return true;
}

bool TransparentWindow::DrawTransWinow() {
    if (transport_window_ == nullptr) {
        return false;
    }
    RECT rect = GetCaptureSourceRect();
    if (::EqualRect(&draw_rect_, &rect)) {
        return true;
    }
    draw_rect_ = rect;
    if (transport_window_) {
        int width = draw_rect_.right - draw_rect_.left;
        int height = draw_rect_.bottom - draw_rect_.top;
        ::SetWindowPos(transport_window_, NULL, draw_rect_.left, draw_rect_.top, width, height,
                       SWP_NOZORDER | SWP_HIDEWINDOW | SWP_NOACTIVATE);
        DrawTransWinowBorder();
        ::SetWindowPos(transport_window_, NULL, draw_rect_.left, draw_rect_.top, width, height,
                       SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE);
    }
    return true;
}

bool TransparentWindow::DestroyTransWindow() {
    if (transport_window_) {
        SendMessage(transport_window_, WM_SYSCOMMAND, SC_CLOSE, NULL);
    }
    UnregisterClass(kClassName, GetModuleHandle(0));
    transport_window_ = nullptr;
    return true;
}

bool TransparentWindow::DrawTransWinowBorder() {
    HDC hdc = GetDC(transport_window_);
    std::shared_ptr<void> release_hdc(nullptr, [&](void*) { ReleaseDC(transport_window_, hdc); });

    HDC mem_dc = ::CreateCompatibleDC(hdc);
    if (!mem_dc) {
        return false;
    }
    
    std::shared_ptr<void> release_mem_dc(nullptr, [&](void*) {
        if (mem_dc) {
            ::DeleteDC(mem_dc);
            mem_dc = nullptr;
        }
    });
    int width = draw_rect_.right - draw_rect_.left;
    int height = draw_rect_.bottom - draw_rect_.top;
    HBITMAP hbitmap = ::CreateCompatibleBitmap(hdc, width, height);
    if (!hbitmap) {
        return false;
    }
    std::shared_ptr<void> release_bitmap(nullptr, [&](void*) {
        if (hbitmap) {
            ::DeleteObject(hbitmap);
            hbitmap = nullptr;
        }
    });
    HBITMAP old_hbitmap = (HBITMAP)::SelectObject(mem_dc, hbitmap);
    Gdiplus::Graphics graphics(mem_dc);
    // clear background
    graphics.Clear(Gdiplus::Color(0, 0, 0));
    Gdiplus::Color gdi_color(kRedColor, kGreenColor, kBlueColor);
    // RGB(0,0,0) is transparent, change RGB(0,0,0) to RGB(1,1,)
    if (kRedColor == 0 && kGreenColor == 0 && kBlueColor == 0) {
        gdi_color = Gdiplus::Color(1, 1, 1);
    }
    Gdiplus::SolidBrush brush(gdi_color);
    graphics.FillRectangle(&brush, 0, 0, width, kBorderWidth);
    graphics.FillRectangle(&brush, 0, height - kBorderWidth, width, kBorderWidth);
    graphics.FillRectangle(&brush, 0, kBorderWidth, kBorderWidth, height - 2 * kBorderWidth);
    graphics.FillRectangle(&brush, width - kBorderWidth, kBorderWidth, kBorderWidth,
                           height - 2 * kBorderWidth);
    
    BLENDFUNCTION bs = {AC_SRC_OVER, 0, kAlpha, AC_SRC_OVER};
    tagPOINT pos{0, 0};
    tagPOINT dstPos = {draw_rect_.left, draw_rect_.top};
    tagSIZE dstSize = {width, height};

    BOOL res = ::UpdateLayeredWindow(transport_window_, hdc, &dstPos, &dstSize, graphics.GetHDC(),
                                     &pos, RGB(0, 0, 0), &bs, ULW_COLORKEY | ULW_ALPHA);
    graphics.ReleaseHDC(mem_dc);
    ::SelectObject(mem_dc, old_hbitmap);
    return true;
}

RECT TransparentWindow::GetCaptureSourceRect() {
    RECT window_rect;
    if (is_window_) {
        ::GetWindowRect(window_id_, &window_rect);
        if (::IsZoomed(window_id_)) {
            window_rect.left += kWindowAeroWidth;
            window_rect.right -= kWindowAeroWidth;
            window_rect.top += kWindowAeroWidth;
            window_rect.bottom -= kWindowAeroWidth;
        }
        else {
            BOOL enable = FALSE;
            DwmGetWindowAttribute(window_id_, DWMWA_NCRENDERING_ENABLED, &enable, sizeof(enable));
            if (VersionHelper::Instance().IsWindows8OrLater() && enable) {
                window_rect.left = window_rect.left + kWindowAeroWidth;
                window_rect.right = window_rect.right - kWindowAeroWidth;
                window_rect.bottom = window_rect.bottom - kWindowAeroWidth;
                /*DwmGetWindowAttribute(window_id_, DWMWA_EXTENDED_FRAME_BOUNDS, &window_rect, sizeof(RECT));*/
            }
            window_rect.left -= kBorderWidth;
            window_rect.right += kBorderWidth;
            window_rect.top -= kBorderWidth;
            window_rect.bottom += kBorderWidth;
        }
    }
    else if (is_display_) {
        WindowRect rect = GetScreenRect(display_id_);
        window_rect.left = rect.left;
        window_rect.top = rect.top;
        window_rect.right = rect.right;
        window_rect.bottom = rect.bottom;
    }
    return window_rect;
}
