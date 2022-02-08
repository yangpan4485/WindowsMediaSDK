#include "video_device_monitor.h"
#include <Dbt.h>
#include <comdef.h>
#include <iostream>
#include <setupapi.h>

#pragma comment(lib, "Setupapi.lib")

static const GUID kCameraGuid = {
    0x65E8773D, 0x8F56, 0x11D0, {0xA3, 0xB9, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96}};
const char kClassName[] = "windows camera monitor";
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        CREATESTRUCT* pcs = (CREATESTRUCT*)lParam;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)pcs->lpCreateParams);
        break;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }
    case WM_CLOSE: {
        DestroyWindow(hWnd);
        break;
    }
    case WM_DEVICECHANGE: {
        VideoDeviceMonitor* video_device_monitor =
            reinterpret_cast<VideoDeviceMonitor*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
        switch (wParam) {
        case DBT_DEVICEARRIVAL: {
            video_device_monitor->JudgeDevice(wParam, lParam, kVideoDeviceAdd);
            break;
        }
        case DBT_DEVICEREMOVECOMPLETE: {
            video_device_monitor->JudgeDevice(wParam, lParam, kVideoDeviceRemove);
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

VideoDeviceMonitor::VideoDeviceMonitor() {
    Start();
}

VideoDeviceMonitor::~VideoDeviceMonitor() {
    Stop();
}

void VideoDeviceMonitor::SetVideoDeviceChangeCallback(VideoDeviceChangeCallback callback) {
    callback_ = callback;
}

bool VideoDeviceMonitor::Start() {
    if (running_) {
        return true;
    }
    if (work_.joinable()) {
        work_.join();
    }
    running_ = true;
    work_ = std::thread([&]() {
        WNDCLASSEX wcex{0};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.lpfnWndProc = WndProc;
        wcex.hInstance = GetModuleHandle(0);
        wcex.lpszClassName = kClassName;
        UnregisterClass(kClassName, GetModuleHandle(0));
        if (!RegisterClassEx(&wcex)) {
            return false;
        }
        hwnd_ = CreateWindowExA(0, kClassName, kClassName, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL,
                                NULL, GetModuleHandle(0), this);
        if (!hwnd_) {
            return false;
        }
        ShowWindow(hwnd_, SW_HIDE);
        UpdateWindow(hwnd_);
        if (!Init()) {
            return false;
        }
        MSG message;
        while (GetMessage(&message, NULL, 0, 0)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        return true;
    });
    return true;
}

bool VideoDeviceMonitor::Init() {
    HDEVNOTIFY hdev_notify = nullptr;
    DEV_BROADCAST_DEVICEINTERFACE notification_filter;
    ZeroMemory(&notification_filter, sizeof(notification_filter));
    notification_filter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    notification_filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    notification_filter.dbcc_classguid = kCameraGuid;
    hdev_notify =
        RegisterDeviceNotification(hwnd_, &notification_filter, DEVICE_NOTIFY_WINDOW_HANDLE);
    if (!hdev_notify) {
        return false;
    }
    return true;
}

bool VideoDeviceMonitor::Stop() {
    running_ = false;
    SendMessage(hwnd_, WM_CLOSE, 0, 0);
    if (work_.joinable()) {
        work_.join();
    }
    return true;
}

UINT VideoDeviceMonitor::JudgeDevice(WPARAM wParam, LPARAM lParam, VideoDeviceEvent event) {
    if (!(DBT_DEVICEARRIVAL == wParam || DBT_DEVICEREMOVECOMPLETE == wParam)) {
        return S_FALSE;
    }
    PDEV_BROADCAST_HDR broadcast_hdr = (PDEV_BROADCAST_HDR)lParam;
    PDEV_BROADCAST_DEVICEINTERFACE broadcast_device_interface;
    switch (broadcast_hdr->dbch_devicetype) {
    case DBT_DEVTYP_DEVICEINTERFACE:
        broadcast_device_interface = (PDEV_BROADCAST_DEVICEINTERFACE)broadcast_hdr;
        if (callback_) {
            std::string id = broadcast_device_interface->dbcc_name;
            auto pos = id.find("{");
            id = id.substr(pos, id.length());
            if (last_device_id_ == id && last_event_ == event) {
                break;
            }
            last_event_ = event;
            last_device_id_ = id;
            callback_(event, broadcast_device_interface->dbcc_name);
        }
        break;
    default:
        break;
    }
    return S_OK;
}