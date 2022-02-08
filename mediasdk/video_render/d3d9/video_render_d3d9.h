#pragma once
#include "video_render.h"
#include <d3d9.h>

class VideoRenderD3d9 : public VideoRender {
public:
    VideoRenderD3d9();
    ~VideoRenderD3d9();

    void RendFrameI420(uint8_t* y_data, uint32_t y_stride, uint8_t* u_data,
        uint32_t u_stride, uint8_t* v_data, uint32_t v_stride,
        uint32_t width, uint32_t height) override;

    void RendFrameNV12(uint8_t* y_data, uint32_t y_stride, uint8_t* uv_data,
        uint32_t uv_stride, uint32_t width, uint32_t height) override;

    void RendFrameNV21(uint8_t* y_data, uint32_t y_stride, uint8_t* vu_data,
        uint32_t vu_stride, uint32_t width, uint32_t height) override;

private:
    void InitRender();
    void CreateSurface();
    void DesrtoySurface();
    void Cleanup();

private:
    CRITICAL_SECTION critical_{};
    IDirect3D9* d3d9_{};
    IDirect3DDevice9* d3d9_device_{};
    IDirect3DSurface9* d3d9_surface_{};
    RECT view_rect_{};
};