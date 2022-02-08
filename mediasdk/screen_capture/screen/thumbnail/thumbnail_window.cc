#include "thumbnail_window.h"

#include <VersionHelpers.h>

ThumbnailWindow::ThumbnailWindow() {}

ThumbnailWindow::~ThumbnailWindow() {
    if (create_thread_.joinable()) {
        create_thread_.join();
    }
}

LRESULT CALLBACK ThumbnailWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }
    case WM_CLOSE: {
        DestroyWindow(hWnd);
        break;
    }
    default:
        return ::DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

bool ThumbnailWindow::Create(const RECT& rect) {
    window_rect_ = rect;
    create_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
    create_thread_ = std::thread([&]() {
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = NULL;
        wcex.hIcon = NULL;
        wcex.hCursor = NULL;
        wcex.hbrBackground = CreateSolidBrush(COLORREF(0));
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = TEXT("ThumbnailWindow");
        wcex.hIconSm = NULL;
        RegisterClassEx(&wcex);

        thumbanil_hwnd_ = ::CreateWindowEx(
            WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, TEXT("ThumbnailWindow"), TEXT("ThumbnailWindow"),
            WS_POPUP, window_rect_.left, window_rect_.top, window_rect_.right - window_rect_.left,
            window_rect_.bottom - window_rect_.top, NULL, NULL, NULL, NULL);
        if (thumbanil_hwnd_ == NULL) {
            create_result_ = false;
            SetEvent(create_event_);
            return;
        }

        ::SetWindowLong(thumbanil_hwnd_, GWL_USERDATA, (LONG)this);
        ::SetWindowLong(thumbanil_hwnd_, GWL_EXSTYLE,
                        ::GetWindowLong(thumbanil_hwnd_, GWL_EXSTYLE) | WS_EX_TRANSPARENT |
                            WS_EX_LAYERED);
        //::SetLayeredWindowAttributes(m_hWnd, RGB(255, 255, 255), 128, LWA_ALPHA |
        // LWA_COLORKEY);
        ::SetTimer(thumbanil_hwnd_, 1, 10, NULL);
        ::SetFocus(thumbanil_hwnd_);
        ::SetWindowPos(thumbanil_hwnd_, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        ::ShowWindow(thumbanil_hwnd_, SW_SHOW);
        ::UpdateWindow(thumbanil_hwnd_);
        create_result_ = true;
        SetEvent(create_event_);

        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0)) {
            //
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (thumbnail_id_ != nullptr) {
            DwmUnregisterThumbnail(thumbnail_id_);
            thumbnail_id_ = nullptr;
        }
    });
    WaitForSingleObject(create_event_, INFINITE);
    CloseHandle(create_event_);
    create_event_ = NULL;
    return create_result_;
}

bool ThumbnailWindow::Destroy() {
    SendMessage(thumbanil_hwnd_, WM_CLOSE, NULL, NULL);
    if (create_thread_.joinable()) {
        create_thread_.join();
    }
    return true;
}

bool ThumbnailWindow::SetTarget(HWND hWndTarget) {
    if (thumbnail_id_ != nullptr) {
        DwmUnregisterThumbnail(thumbnail_id_);
        thumbnail_id_ = nullptr;
    }
    if (DwmRegisterThumbnail(thumbanil_hwnd_, hWndTarget, &thumbnail_id_) != S_OK) {
        return false;
    }
    int width = window_rect_.right - window_rect_.left;
    int height = window_rect_.bottom - window_rect_.top;
    SIZE size;
    if (DwmQueryThumbnailSourceSize(thumbnail_id_, &size) != S_OK) {
        DwmUnregisterThumbnail(thumbnail_id_);
        thumbnail_id_ = nullptr;
        return false;
    }
    int offset_x = 0;
    int offset_y = 0;
    if (IsWindows8OrGreater()) {
        RECT rect;
        // 去掉边缘毛玻璃效果
        GetWindowRect(hWndTarget, &rect);
        if (IsZoomed(hWndTarget)) {
            rect.left += 8;
            rect.right -= 8;
            rect.top += 8;
            rect.bottom -= 8;
            size.cx = rect.right - rect.left;
            size.cy = rect.bottom - rect.top;
            offset_x = 8;
            offset_y = 8;
        } else if (IsIconic(hWndTarget)) {
            WINDOWPLACEMENT wp = {};
            wp.length = sizeof(WINDOWPLACEMENT);
            GetWindowPlacement(hWndTarget, &wp);
            int restoreW = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
            int restoreH = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
            if ((size.cx != restoreW) || (size.cy != restoreH)) {
                size.cx -= 16;
                size.cy -= 18;
                offset_x = 8;
                offset_y = 8;
            } else {
                RECT rect2 = rect;
                DwmGetWindowAttribute(hWndTarget, DWMWA_EXTENDED_FRAME_BOUNDS, &rect2,
                                      sizeof(RECT));
                //
                int w1 = rect.right - rect.left;
                int h1 = rect.bottom - rect.top;
                int w2 = rect2.right - rect2.left;
                int h2 = rect2.bottom - rect2.top;
                size.cx -= (w1 - w2);
                size.cy -= (h1 - h2);
                // Solve the problem of the minimizing the bottom of window has black border
                if ((w1 - w2 != 0) || (h1 - h2 != 0)) {
                    size.cy -= 8;
                }
                offset_x = rect2.left - rect.left;
                offset_y = rect2.top - rect.top;
            }
        } else {
            RECT rect2 = rect;
            DwmGetWindowAttribute(hWndTarget, DWMWA_EXTENDED_FRAME_BOUNDS, &rect2, sizeof(RECT));
            offset_x = rect2.left - rect.left;
            offset_y = rect2.top - rect.top;
        }
    }

    DWM_THUMBNAIL_PROPERTIES properties;
    properties.dwFlags = DWM_TNP_RECTDESTINATION | DWM_TNP_RECTSOURCE | DWM_TNP_VISIBLE;
    properties.rcSource.left = offset_x;
    properties.rcSource.top = offset_y;
    properties.rcSource.right = offset_x + size.cx;
    properties.rcSource.bottom = offset_y + size.cy;
    properties.fVisible = TRUE;
    properties.rcDestination.left = 0;
    properties.rcDestination.top = 0;
    properties.rcDestination.right = width;
    properties.rcDestination.bottom = height;
    properties.rcSource.right = offset_x + size.cx;
    properties.rcSource.bottom = offset_y + size.cy;
    if (DwmUpdateThumbnailProperties(thumbnail_id_, &properties) != S_OK) {
        DwmUnregisterThumbnail(thumbnail_id_);
        thumbnail_id_ = nullptr;
        return false;
    }
    return true;
}