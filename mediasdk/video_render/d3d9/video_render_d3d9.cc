#include "video_render_d3d9.h"

#include <iostream>

VideoRenderD3d9::VideoRenderD3d9() {

}

VideoRenderD3d9::~VideoRenderD3d9() {
    DesrtoySurface();
    Cleanup();
}

void VideoRenderD3d9::RendFrameI420(uint8_t* y_data, uint32_t y_stride, uint8_t* u_data,
    uint32_t u_stride, uint8_t* v_data, uint32_t v_stride,
    uint32_t width, uint32_t height) {
    if (frame_width_ != width || frame_height_ != height || CheckWindowSizeChange()) {
        frame_width_ = width;
        frame_height_ = height;
        view_rect_.left = 0;
        view_rect_.top = 0;
        view_rect_.right = width;
        view_rect_.bottom = height;
        InitRender();
        CreateSurface();
    }
    D3DLOCKED_RECT d3d_rect;
    auto hr = d3d9_surface_->LockRect(&d3d_rect, NULL, D3DLOCK_DONOTWAIT);
    if (FAILED(hr)) {
        std::cout << "lock rect error" << std::endl;
        return;
    }

    int y_pitch = d3d_rect.Pitch;
    int v_pitch = y_pitch / 2;
    int u_pitch = y_pitch / 2;

    uint8_t* y_dest = (uint8_t*)d3d_rect.pBits;
    uint8_t* v_dest = y_dest + frame_height_ * y_pitch;
    uint8_t* u_dest = v_dest + frame_height_ / 2 * v_pitch;

    for (uint32_t i = 0; i < frame_height_; i++) {
        memcpy(y_dest + i * y_pitch, y_data + i * frame_width_, frame_width_);
    }

    for (uint32_t i = 0; i < frame_height_ / 2; i++) {
        memcpy(v_dest + i * v_pitch, v_data + i * frame_width_ / 2, frame_width_ / 2);
    }

    for (uint32_t i = 0; i < frame_height_ / 2; i++) {
        memcpy(u_dest + i * u_pitch, u_data + i * frame_width_ / 2, frame_width_ / 2);
    }

    hr = d3d9_surface_->UnlockRect();
    if (FAILED(hr)) {
        std::cout << "unlock rect error" << std::endl;
        return;
    }
    d3d9_device_->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    d3d9_device_->BeginScene();
    IDirect3DSurface9* d3d_surface = NULL;
    d3d9_device_->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &d3d_surface);
    d3d9_device_->StretchRect(d3d9_surface_, NULL, d3d_surface, &view_rect_, D3DTEXF_LINEAR);
    d3d9_device_->EndScene();
    d3d9_device_->Present(NULL, NULL, NULL, NULL);
    d3d_surface->Release();
}

void VideoRenderD3d9::RendFrameNV12(uint8_t* y_data, uint32_t y_stride, uint8_t* uv_data,
    uint32_t uv_stride, uint32_t width, uint32_t height) {
}

void VideoRenderD3d9::RendFrameNV21(uint8_t* y_data, uint32_t y_stride, uint8_t* vu_data,
    uint32_t vu_stride, uint32_t width, uint32_t height) {
}

void VideoRenderD3d9::InitRender() {
    std::cout << "init render" << std::endl;
    InitializeCriticalSection(&critical_);
    Cleanup();

    d3d9_ = Direct3DCreate9(D3D_SDK_VERSION);
    if (d3d9_ == nullptr) {
        return;
    }

    RECT r;
    GetClientRect(render_window_, &r);
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.hDeviceWindow = (HWND)render_window_;
    d3dpp.Flags = D3DPRESENTFLAG_VIDEO;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferWidth = frame_width_;
    d3dpp.BackBufferHeight = frame_height_;

    HRESULT hr = d3d9_->CreateDevice(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, render_window_,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3d9_device_);
}

void VideoRenderD3d9::CreateSurface() {
    if (d3d9_device_ == nullptr) {
        return;
    }
    HRESULT hr = d3d9_device_->CreateOffscreenPlainSurface(
        frame_width_, frame_height_, (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2'),
        D3DPOOL_DEFAULT, &d3d9_surface_, NULL);
    if (FAILED(hr)) {
        std::cout << "create plain surface error" << std::endl;
        return;
    }
    std::cout << "create surface" << std::endl;
}

void VideoRenderD3d9::DesrtoySurface() {
    EnterCriticalSection(&critical_);
    if (d3d9_surface_) {
        d3d9_surface_->Release();
        d3d9_surface_ = nullptr;
    }
    LeaveCriticalSection(&critical_);
}

void VideoRenderD3d9::Cleanup() {
    EnterCriticalSection(&critical_);
    if (d3d9_surface_) {
        d3d9_surface_->Release();
        d3d9_surface_ = nullptr;
    }
    if (d3d9_device_) {
        d3d9_device_->Release();
        d3d9_device_ = nullptr;
    }
    if (d3d9_) {
        d3d9_->Release();
        d3d9_ = nullptr;
    }
    LeaveCriticalSection(&critical_);
}