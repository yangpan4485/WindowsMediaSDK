#pragma once
#include <Windows.h>
#include <cstdint>

enum PixFormat {
    I420,
    NV12,
    NV21
};

class VideoRender {
public:
    VideoRender();
    virtual ~VideoRender();

    void SetWindow(HWND window);

    virtual void RendFrame(uint8_t* data, uint32_t width, uint32_t height);

    virtual void RendFrameI420(uint8_t* y_data, uint32_t y_stride, uint8_t* u_data,
                               uint32_t u_stride, uint8_t* v_data, uint32_t v_stride,
                               uint32_t width, uint32_t height);

    virtual void RendFrameNV12(uint8_t* y_data, uint32_t y_stride, uint8_t* uv_data,
                               uint32_t uv_stride, uint32_t width, uint32_t height);

    virtual void RendFrameNV21(uint8_t* y_data, uint32_t y_stride, uint8_t* vu_data,
                               uint32_t vu_stride, uint32_t width, uint32_t height);

    bool CheckWindowSizeChange();

protected:
    HWND render_window_{};
    uint32_t frame_width_{};
    uint32_t frame_height_{};
    uint32_t window_width_{};
    uint32_t window_height_{};
    PixFormat pix_format_{};
};