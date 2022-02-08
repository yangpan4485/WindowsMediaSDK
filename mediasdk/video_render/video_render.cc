#include "video_render.h"

VideoRender::VideoRender() {}

VideoRender::~VideoRender() {}

void VideoRender::SetWindow(HWND window) {
    render_window_ = window;
}

void VideoRender::RendFrame(uint8_t* data, uint32_t width, uint32_t height) {

}

void VideoRender::RendFrameI420(uint8_t* y_data, uint32_t y_stride, uint8_t* u_data,
                                uint32_t u_stride, uint8_t* v_data, uint32_t v_stride,
                                uint32_t width, uint32_t height) {}

void VideoRender::RendFrameNV12(uint8_t* y_data, uint32_t y_stride, uint8_t* uv_data,
                                uint32_t uv_stride, uint32_t width, uint32_t height) {}

void VideoRender::RendFrameNV21(uint8_t* y_data, uint32_t y_stride, uint8_t* vu_data,
                                uint32_t vu_stride, uint32_t width, uint32_t height) {}

bool VideoRender::CheckWindowSizeChange() {
    RECT rect;
    ::GetClientRect(render_window_, &rect);
    int client_width = (rect.right - rect.left);
    int client_height = (rect.bottom - rect.top);
    if (client_width != window_width_ || client_height != window_height_) {
        window_width_ = client_width;
        window_height_ = client_height;
        return true;
    }
    return false;
}