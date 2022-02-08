#include "window_utils.h"

#include <wrl/client.h>
#include <dwmapi.h>
#include <shlobj.h>
#include <dxgi1_2.h>
#include <d3d11.h>

#include "screen_common.h"
#include "common/version_helper.h"
#include "string_utils.h"

//#include <VersionHelpers.h>
//#include <d3d11.h>
//#include <dwmapi.h>
//#include <dxgi1_2.h>
//#include <gdiplus.h>
//#include <iostream>
//#include <shlobj.h>
//#include <time.h>
//#include <wrl/client.h>
//
//#include "mag/screen_capture_mag.h"
//#include "mag/thumbnail_window.h"
//// #include "common/video_frame.h"
//#include "mag/mag_factory.h"

//#pragma comment(lib, "dwmapi.lib")
//#pragma comment(lib, "Gdiplus.lib")
//#pragma comment(lib, "D3D11.lib")
//#pragma comment(lib, "wtsapi32.lib")

bool IsWindowValidAndVisible(HWND window) {
    return IsWindow(window) && IsWindowVisible(window) && !IsIconic(window);
}

bool IsWindowOnCurrentDesktop(HWND hwnd) {
    BOOL on_current_desktop = TRUE;
    Microsoft::WRL::ComPtr<IVirtualDesktopManager> virtual_desktop_manager;
    if (VersionHelper::Instance().IsWindows10OrLater()) {
        HRESULT hr = ::CoCreateInstance(__uuidof(VirtualDesktopManager), nullptr, CLSCTX_ALL,
            IID_PPV_ARGS(&virtual_desktop_manager));
        if (FAILED(hr)) {
            return on_current_desktop;
        }
        virtual_desktop_manager->IsWindowOnCurrentVirtualDesktop(hwnd, &on_current_desktop);
    }
    return on_current_desktop;
}

bool IsWindowCloaked(HWND hwnd) {
    // LoadLibraryW(L"dwmapi.dll"); 防止操作系统不支持
    int res = 0;
    DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &res, sizeof(res));
    return res != 0;
}

bool IsWindowsSysmtemWnd(HWND hwnd) {
    if (!VersionHelper::Instance().IsWindows8OrLater()) {
        return false;
    }
    char className[2048] = {0};
    GetClassName(hwnd, className, 2048);
    if (strcmp(className, "Windows.UI.Core.CoreWindow") == 0) {
        return true;
    }
    if (strcmp(className, "ApplicationFrameWindow")) {
        return false;
    }
    std::vector<HWND> windows;
    GetChildWindows(hwnd, windows);
    bool bCheckChild = false;
    for (const auto& window : windows) {
        memset(className, 0, 2048);
        GetClassName(window, className, 2048);
        if (strcmp(className, "Windows.UI.Core.CoreWindow") == 0) {
            bCheckChild = true;
            break;
        }
    }
    if (bCheckChild) {
        return false;
    }
    // if isconic is false the process is suspended
    if (!IsIconic(hwnd)) {
        return true;
    }
    return false;
}

// 检查窗口是否在 50 ms 内响应消息
bool IsWindowResponding(HWND window) {
    const UINT uTimeoutMs = 50;
    return SendMessageTimeout(window, WM_NULL, 0, 0, SMTO_ABORTIFHUNG, uTimeoutMs, nullptr);
}

bool IsWindowVisibleOnCurrentDesktop(HWND hwnd) {
    return !IsWindowCloaked(hwnd);
}

bool CanSafelyMakeBlockingCalls(HWND hwnd) {
    DWORD process_id;
    GetWindowThreadProcessId(hwnd, &process_id);
    if (process_id != GetCurrentProcessId()) {
        return true;
    }
    return false;
}

BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam) {
    std::vector<HWND>* windows = (std::vector<HWND>*)lParam;
    if (!windows) {
        return FALSE;
    }
    windows->push_back(hWnd);
    return TRUE;
}

void GetChildWindows(HWND windowID, std::vector<HWND>& windows) {
    windows.clear();
    ::EnumChildWindows(windowID, EnumChildProc, (LPARAM)&windows);
}

BOOL CALLBACK EnumWindowProc(HWND hwnd, LPARAM param) {
    std::vector<WindowInfo>* window_list = reinterpret_cast<std::vector<WindowInfo>*>(param);
    if (!IsWindowVisible(hwnd)) {
        return TRUE;
    }
    if (IsWindowCloaked(hwnd)) {
        return TRUE;
    }
    HWND owner = GetWindow(hwnd, GW_OWNER);
    LONG exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    if (owner && !(exstyle & WS_EX_APPWINDOW)) {
        return TRUE;
    }
    const size_t kMaxClassNameLength = 256;
    WCHAR class_name[kMaxClassNameLength] = L"";
    const int class_name_length = GetClassNameW(hwnd, class_name, kMaxClassNameLength);
    if (class_name_length < 1) {
        return TRUE;
    }
    // Skip Program Manager window.
    if (wcscmp(class_name, L"Progman") == 0) {
        return TRUE;
    }
    if (wcscmp(class_name, L"Button") == 0) {
        return TRUE;
    }
    if (IsWindowsSysmtemWnd(hwnd)) {
        return TRUE;
    }
    if (!IsWindowResponding(hwnd)) {
        return TRUE;
    }
    WindowInfo window;
    window.hwnd = hwnd;
    if (CanSafelyMakeBlockingCalls(hwnd)) {
        const size_t kTitleLength = 500;
        WCHAR window_title[kTitleLength] = L"";
        DWORD window_pid = 0;
        ::GetWindowThreadProcessId(hwnd, &window_pid);
        if (window_pid == ::GetCurrentProcessId()) {
            if (InternalGetWindowText(hwnd, window_title, kTitleLength) > 0) {
                window.window_title = utils::UnicodeToUtf8(window_title);
            }
        }
        else if (GetWindowTextLength(hwnd) != 0 &&
            GetWindowTextW(hwnd, window_title, kTitleLength) > 0) {
            window.window_title = utils::UnicodeToUtf8(window_title);
        }
    }
    if (window.window_title.empty()) {
        return TRUE;
    }
    window_list->push_back(std::move(window));
    return TRUE;
}

BOOL CALLBACK EnumMonitorProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor,
    LPARAM dwData) {
    std::vector<HMONITOR>* monitors = (std::vector<HMONITOR>*)dwData;
    monitors->push_back(hMonitor);
    return TRUE;
}

void EnumMonitor(std::vector<HMONITOR>& monitor_list) {
    monitor_list.clear();
    EnumDisplayMonitors(NULL, NULL, EnumMonitorProc, (LPARAM)&monitor_list);
}

WindowRect GetScreenRect(int screen_id) {
    DISPLAY_DEVICEW device;
    device.cb = sizeof(device);
    BOOL result = EnumDisplayDevicesW(NULL, screen_id, &device, 0);
    DEVMODEW device_mode;
    device_mode.dmSize = sizeof(device_mode);
    device_mode.dmDriverExtra = 0;
    result = EnumDisplaySettingsExW(device.DeviceName, ENUM_CURRENT_SETTINGS, &device_mode, 0);
    WindowRect window_rect;
    window_rect.left = device_mode.dmPosition.x;
    window_rect.top = device_mode.dmPosition.y;
    window_rect.right = window_rect.left + device_mode.dmPelsWidth;
    window_rect.bottom = window_rect.top + device_mode.dmPelsHeight;
    return window_rect;
}

bool AeroEnabled() {
    BOOL enabled = FALSE;
    HRESULT ret = DwmIsCompositionEnabled(&enabled);
    if (ret != S_OK) {
        return FALSE;
    }
    return enabled;
}

bool IsRunOnIntelGPU(HRESULT& hr) {
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
                                       D3D_FEATURE_LEVEL_9_1 };
    UINT featureLevelsNum = ARRAYSIZE(featureLevels);
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    D3D_FEATURE_LEVEL featureLevel;
    hr = E_FAIL;
    hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevels, featureLevelsNum,
        D3D11_SDK_VERSION, device.GetAddressOf(), &featureLevel, context.GetAddressOf());
    if (!SUCCEEDED(hr)) {
        return false;
    }
    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    hr = device.As(&dxgiDevice);
    if (!SUCCEEDED(hr)) {
        return false;
    }
    Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
    hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(dxgiAdapter.GetAddressOf()));
    if (!SUCCEEDED(hr)) {
        return false;
    }
    UINT output = 0;
    Microsoft::WRL::ComPtr<IDXGIOutput> dxgiOutput;
    hr = dxgiAdapter->EnumOutputs(output, dxgiOutput.GetAddressOf());
    if (!SUCCEEDED(hr)) {
        return false;
    }
    Microsoft::WRL::ComPtr<IDXGIOutput1> dxgiOutput1;
    hr = dxgiOutput.As(&dxgiOutput1);
    if (!SUCCEEDED(hr)) {
        return false;
    }
    Microsoft::WRL::ComPtr<IDXGIOutputDuplication> outputDupl;
    hr = dxgiOutput1->DuplicateOutput(static_cast<IUnknown*>(device.Get()), outputDupl.GetAddressOf());
    if (!SUCCEEDED(hr) && DXGI_ERROR_UNSUPPORTED == hr) {
        return false;
    }
    return true;
}

bool IsEqualRect(const WindowRect& rect1, const WindowRect& rect2) {
    if (rect1.left == rect2.left && rect1.right == rect2.right
        && rect1.top == rect2.top && rect1.bottom == rect2.bottom) {
        return true;
    }
    return false;

}

std::vector<HWND> GetOverWindow(HWND target_hwnd) {
    if (!target_hwnd) {
        return std::vector<HWND>();
    }
    RECT target_window_rect{};
    RECT window_rect{};
    RECT over_rect{};
    std::vector<HWND> over_window{};
    GetWindowRect(target_hwnd, &target_window_rect);
    auto hwnd = FindWindowEx(GetDesktopWindow(), nullptr, nullptr, nullptr);
    while (hwnd != nullptr && hwnd != target_hwnd) {
        if (target_hwnd != GetWindow(hwnd, GW_OWNER) && IsWindowVisible(hwnd)) {
            GetWindowRect(hwnd, &window_rect);
            if (::IntersectRect(&over_rect, &window_rect, &target_window_rect)) {
                over_window.emplace_back(hwnd);
            }
        }
        hwnd = FindWindowEx(GetDesktopWindow(), hwnd, nullptr, nullptr);
    }
    return over_window;
}