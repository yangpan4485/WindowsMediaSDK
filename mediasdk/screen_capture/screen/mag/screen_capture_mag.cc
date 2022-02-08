#include "screen_capture_mag.h"

#include <iostream>

#include "screen/hook/d3d9_hook.h"
#include "screen/window_utils.h"

BOOL CALLBACK ScreenCaptureMag::OnMagImageScalingCallback(HWND hwnd, void* srcdata,
                                                          MAGIMAGEHEADER srcheader, void* destdata,
                                                          MAGIMAGEHEADER destheader, RECT unclipped,
                                                          RECT clipped, HRGN dirty) {
    if (srcheader.format != GUID_WICPixelFormat32bppRGBA &&
        srcheader.format != GUID_WICPixelFormat32bppBGR) {
        WCHAR szBuffer[256] = {0};
        StringFromGUID2(srcheader.format, szBuffer, 256);
    }
    if ((srcheader.width * srcheader.height * 4) != srcheader.cbSize) {
        return FALSE;
    }
    ScreenCaptureMag* magEngine =
        reinterpret_cast<ScreenCaptureMag*>(::GetWindowLongPtr(::GetParent(hwnd), GWLP_USERDATA));
    magEngine->OnMagCallback(srcheader.width, srcheader.height, (uint8_t*)srcdata);
    return TRUE;
}

LRESULT CALLBACK ScreenCaptureMag::HostWindowProc(HWND hWnd, UINT message, WPARAM wParam,
                                                  LPARAM lParam) {
    if (message == WM_NCCREATE) {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        ScreenCaptureMag* screen_capture_mag = static_cast<ScreenCaptureMag*>(lpcs->lpCreateParams);
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(screen_capture_mag));
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

HRESULT CALLBACK ScreenCaptureMag::StretchRectHook(
    IDirect3DDevice9Ex* d3d_device, IDirect3DSurface9* d3d_src_surface, const RECT* src_rect,
    IDirect3DSurface9* d3d_dest_surface, const RECT* dest_rect, D3DTEXTUREFILTERTYPE filter) {
    HookStretchRectFunc* stretch_rect_hook_func = D3D9Hook::GetInstance().GetStretchRectHookFunc();
    D3DDEVICE_CREATION_PARAMETERS param{};
    d3d_device->GetCreationParameters(&param);
    if (param.hFocusWindow == nullptr) {
        return (*stretch_rect_hook_func)(d3d_device, d3d_src_surface, src_rect, d3d_dest_surface,
                                         dest_rect, filter);
    }
    ScreenCaptureMag* screen_capture_mag =
        reinterpret_cast<ScreenCaptureMag*>(::GetWindowLongPtr(param.hFocusWindow, GWLP_USERDATA));
    if (!screen_capture_mag) {
        return (*stretch_rect_hook_func)(d3d_device, d3d_src_surface, src_rect, d3d_dest_surface,
                                         dest_rect, filter);
    }
    screen_capture_mag->OnStretchRect(d3d_device, d3d_src_surface);
    return D3DERR_INVALIDCALL;
}

ScreenCaptureMag::ScreenCaptureMag() {
    MagInitialize();
}

ScreenCaptureMag::~ScreenCaptureMag() {
    MagUninitialize();
    Uninit();
}

std::shared_ptr<VideoFrame> ScreenCaptureMag::CaptureWindow(const WindowInfo& window_info) {
    enable_mag_callback_ = false;
    enable_d3d9_hook_ = true;
    if (!init_) {
        Init();
    }
    std::vector<HWND> over_window = GetOverWindow(window_info.hwnd);
    if (over_window_.empty()) {
        over_window_.swap(over_window);
        for (size_t i = 0; i < over_window_.size(); ++i) {
            over_window_map_[over_window_[i]] = true;
        }
    } else {
        for (auto iter = over_window_.begin(); iter != over_window_.end(); ++iter) {
            if (over_window_map_[*iter]) {
                // TODO
            }
        }
    }
    SetFilterWindow(over_window);
    RECT window_rect{};
    ::GetWindowRect(window_info.hwnd, &window_rect);
    return CaptureFrame(window_rect, true);
}

std::shared_ptr<VideoFrame> ScreenCaptureMag::CaptureScreen(const ScreenInfo& screen_info) {
    if (screen_info.is_primary) {
        enable_mag_callback_ = true;
        enable_d3d9_hook_ = false;
    } else {
        enable_mag_callback_ = false;
        enable_d3d9_hook_ = true;
    }
    if (mag_screen_info_.display_id != screen_info.display_id ||
        mag_screen_info_.screen_rect.left != screen_info.screen_rect.left ||
        mag_screen_info_.screen_rect.top != screen_info.screen_rect.top ||
        mag_screen_info_.screen_rect.right != screen_info.screen_rect.right ||
        mag_screen_info_.screen_rect.bottom != screen_info.screen_rect.bottom) {
        mag_screen_info_ = screen_info;
        Uninit();
    }
#if 0
    RECT rect{ screen_info.screen_rect.left, screen_info.screen_rect.top,
              screen_info.screen_rect.left + 1920, screen_info.screen_rect.top + 1080 };
    capture_rect_ = rect;
#else
    RECT rect{screen_info.screen_rect.left, screen_info.screen_rect.top,
              screen_info.screen_rect.right, screen_info.screen_rect.bottom};
    capture_rect_ = rect;
#endif
    if (!init_) {
        Init();
        // UpdateMagWindowPos(rect);
    }
    return CaptureFrame(rect, true);
}

std::shared_ptr<VideoFrame> ScreenCaptureMag::GetThumbnailWindowImage(RECT thumbnail_window_rect) {
    enable_mag_callback_ = false;
    enable_d3d9_hook_ = true;
    capture_thumbnail_ = true;
    if (!::EqualRect(&last_thumbnail_rect_, &thumbnail_window_rect)) {
        last_thumbnail_rect_ = thumbnail_window_rect;
        init_ = false;
    }
    if (!init_) {
        if (!Init()) {
            return nullptr;
        }
    }
    return CaptureFrame(thumbnail_window_rect, false);
}

bool ScreenCaptureMag::Init() {
    MagUninitialize();
    MagInitialize();
    if (enable_d3d9_hook_) {
        D3D9Hook::GetInstance().HookStretchRect(StretchRectHook);
    }
    if (!CreateMagWindow()) {
        std::cout << "create window failed" << std::endl;
        return false;
    }
    if (enable_mag_callback_) {
        if (!MagSetImageScalingCallback(control_window_, &OnMagImageScalingCallback)) {
            std::cout << "MagSetImageScalingCallback failed: " << GetLastError() << std::endl;
            DestroyMagWindow();
            return false;
        }
    }
    SetFilterWindow(ignore_window_list_);
    init_ = true;
    return true;
}

bool ScreenCaptureMag::Uninit() {
    if (!init_) {
        return true;
    }
    if (enable_mag_callback_) {
        MagSetImageScalingCallback(control_window_, NULL);
    }
    if (enable_d3d9_hook_) {
        D3D9Hook::GetInstance().UnhookStretchRect(StretchRectHook);
    }
    if (enable_d3d9_hook_) {
        if (render_surface_ != NULL) {
            render_surface_->Release();
            render_surface_ = NULL;
        }
        if (off_screen_surface_ != NULL) {
            off_screen_surface_->Release();
            off_screen_surface_ = NULL;
        }
    }
    DestroyMagWindow();
    init_ = false;
    return true;
}

void ScreenCaptureMag::OnMagCallback(int width, int height, uint8_t* data) {
    if (capture_screen_) {
        capture_frame_.reset(new VideoFrame(capture_width_, capture_height_, kFrameTypeARGB, true));
    } else {
        capture_frame_.reset(
            new VideoFrame(capture_width_, capture_height_, kFrameTypeARGB, false));
    }
    if ((capture_width_ == width) && (capture_height_ == height)) {
        memcpy(capture_frame_->GetData(), data, capture_width_ * capture_height_ * 4);
        capture_result_ = true;
        return;
    }
    if ((capture_width_ <= width) && (capture_height_ <= height)) {
        uint8_t* dst = capture_frame_->GetData();
        int dst_pitch = capture_width_ * 4;
        uint8_t* src = (uint8_t*)data;
        int src_pitch = width * 4;
        for (int i = 0; i < capture_height_; i++) {
            memcpy(dst, src, dst_pitch);
            dst += dst_pitch;
            src += src_pitch;
        }
    }
}

std::shared_ptr<VideoFrame> ScreenCaptureMag::CaptureFrame(const RECT& rect, bool capture_cursor) {
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    if (capture_width_ != width || capture_height_ != height) {
        capture_width_ = width;
        capture_height_ = height;
        SetWindowPos(control_window_, nullptr, 0, 0, width, height, SWP_NOMOVE);
        MagSetWindowSource(control_window_, rect);
    }
    if (enable_cursor_ != capture_cursor) {
        enable_cursor_ = capture_cursor;
        UpdateCursor();
    }
    capture_result_ = false;
    if (!MagSetWindowSource(control_window_, rect)) {
        std::cout << "MagSetWindowSource failed" << std::endl;
        return false;
    }
    if (capture_result_) {
        return capture_frame_;
    }
    return nullptr;
}

void ScreenCaptureMag::UpdateCursor() {
    HWND hwnd = control_window_;
    if (enable_cursor_) {
        ::SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) | MS_SHOWMAGNIFIEDCURSOR);
    } else {
        ::SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~MS_SHOWMAGNIFIEDCURSOR);
    }
}

void ScreenCaptureMag::SetFilterWindow(const std::vector<HWND>& filter_window) {
    if (filter_window.empty()) {
        MagSetWindowFilterList(control_window_, MW_FILTERMODE_EXCLUDE, 0, NULL);
        return;
    }
    MagSetWindowFilterList(control_window_, MW_FILTERMODE_EXCLUDE, filter_window.size(),
                           (HWND*)&filter_window[0]);
}

void ScreenCaptureMag::OnStretchRect(IDirect3DDevice9Ex* d3d9_device,
                                     IDirect3DSurface9* d3d9_surface) {
    if (CheckSizeUpdate()) {
        if (render_surface_) {
            render_surface_->Release();
            render_surface_ = nullptr;
        }
        if (off_screen_surface_) {
            off_screen_surface_->Release();
            off_screen_surface_ = nullptr;
        }
        HRESULT ret = d3d9_device->CreateRenderTarget(frame_width_, frame_height_, D3DFMT_A8R8G8B8,
                                                      D3DMULTISAMPLE_NONE, 0, false,
                                                      &render_surface_, nullptr);
        ret = d3d9_device->CreateOffscreenPlainSurface(frame_width_, frame_height_, D3DFMT_A8R8G8B8,
                                                       D3DPOOL_SYSTEMMEM, &off_screen_surface_,
                                                       nullptr);
        if (!off_screen_surface_) {
            render_surface_->Release();
            render_surface_ = nullptr;
        }
    }
    (*D3D9Hook::GetInstance().GetStretchRectHookFunc())(d3d9_device, d3d9_surface, nullptr,
                                                        render_surface_, nullptr, D3DTEXF_NONE);
    d3d9_device->GetRenderTargetData(render_surface_, off_screen_surface_);
    if (capture_screen_ || capture_thumbnail_) {
        capture_frame_.reset(new VideoFrame(capture_width_, capture_height_, kFrameTypeARGB, true));
    } else {
        capture_frame_.reset(
            new VideoFrame(capture_width_, capture_height_, kFrameTypeARGB, false));
    }
    // Dev C++ window click mini button the window is invisible
    if (capture_hwnd_ && !::IsWindowVisible(capture_hwnd_)) {
        return;
    }
    if (capture_hwnd_ && ::IsIconic(capture_hwnd_)) {
        return;
    }
    D3DLOCKED_RECT lockedRect{};
    off_screen_surface_->LockRect(&lockedRect, nullptr, 0);
    if (!capture_frame_) {
        std::cout << "capture_frame_ is nullptr" << std::endl;
    }
    memcpy(capture_frame_->GetData(), lockedRect.pBits, frame_width_ * frame_height_ * 4);
    off_screen_surface_->UnlockRect();
    capture_result_ = true;
}

bool ScreenCaptureMag::CheckSizeUpdate() {
    int width = 0;
    int height = 0;
    CalcLastFrameSize(width, height);
    if (frame_width_ != width || frame_height_ != height) {
        frame_width_ = width;
        frame_height_ = height;
        return true;
    }
    return false;
}

void ScreenCaptureMag::CalcLastFrameSize(int& width, int& height) {
    width = capture_width_;
    height = capture_height_;
}

void ScreenCaptureMag::UpdateMagWindowPos(const RECT& rect) {
    if (capture_rect_.left != rect.left || capture_rect_.right != rect.right ||
        capture_rect_.top != rect.top || capture_rect_.bottom != rect.bottom) {
        capture_rect_ = rect;
        ::SetWindowPos(host_window_, nullptr, capture_rect_.left, capture_rect_.top,
                       capture_rect_.right - capture_rect_.left,
                       capture_rect_.bottom - capture_rect_.top, 0);
    }
}

bool ScreenCaptureMag::CreateMagWindow() {
#if 0
    WNDCLASS wc = { 0 };
    wc.lpszClassName = "ScreenCapturerWinMagnifierHost";
    wc.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = NULL;
    if (RegisterClass(&wc) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    {
        return false;
    }
    static wchar_t kMagnifierHostClass[] = L"ScreenCapturerWinMagnifierHost";
    static wchar_t kHostWindowName[] = L"MagnifierHost";
    host_window_ = CreateWindowExW(WS_EX_LAYERED, kMagnifierHostClass, kHostWindowName, 0, 0,
        0, 0, 0, nullptr, nullptr, nullptr, nullptr);
    if (!host_window_) {
        MagUninitialize();
        return false;
    }
    control_window_ = CreateWindow(WC_MAGNIFIER, "MagnifierWindow", WS_CHILD | WS_VISIBLE, 
        0, 0, 1920, 1080, host_window_, NULL, NULL, NULL);
    if (!control_window_) {
        MagUninitialize();
        return false;
    }
    ShowWindow(control_window_, SW_HIDE);
    ::SetWindowLongPtr(control_window_, GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(this));
    return true;
#else
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = ScreenCaptureMag::HostWindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = NULL;
    wcex.hIcon = NULL;
    wcex.hCursor = NULL;
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = TEXT("MagHost");
    wcex.hIconSm = NULL;
    RegisterClassEx(&wcex);
    int width = capture_rect_.right - capture_rect_.left;
    int height = capture_rect_.bottom - capture_rect_.top;
    host_window_ = CreateWindowEx(0, "MagHost", "MagHost",
                                  WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, capture_rect_.left,
                                  capture_rect_.top, capture_rect_.right - capture_rect_.left,
                                  capture_rect_.bottom - capture_rect_.top, NULL, NULL, NULL, this);
    SetWindowLong(host_window_, GWL_EXSTYLE,
                  ::GetWindowLong(host_window_, GWL_EXSTYLE) | WS_EX_LAYERED);
    control_window_ =
        ::CreateWindowEx(0, WC_MAGNIFIER, "MagControl", WS_CHILD | MS_SHOWMAGNIFIEDCURSOR, 0, 0,
                         capture_rect_.right - capture_rect_.left,
                         capture_rect_.bottom - capture_rect_.top, host_window_, NULL, NULL, NULL);
    if (control_window_ == NULL) {
        DestroyWindow(host_window_);
        host_window_ = NULL;
        return false;
    }
    ::SetWindowLongPtr(control_window_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
#endif
    return true;
}

bool ScreenCaptureMag::DestroyMagWindow() {
    if (host_window_) {
        DestroyWindow(host_window_);
        host_window_ = nullptr;
    }
    if (control_window_) {
        DestroyWindow(control_window_);
        control_window_ = nullptr;
    }
    return true;
}
