#pragma once

#include <GL/glew.h>
#include <GL/wglew.h>

#include "video_render.h"

class VideoRenderOpenGL : public VideoRender {
public:
    VideoRenderOpenGL();
    ~VideoRenderOpenGL();

    void RendFrameI420(uint8_t* y_data, uint32_t y_stride, uint8_t* u_data, uint32_t u_stride,
                       uint8_t* v_data, uint32_t v_stride, uint32_t width,
                       uint32_t height) override;

    void RendFrameNV12(uint8_t* y_data, uint32_t y_stride, uint8_t* uv_data, uint32_t uv_stride,
                       uint32_t width, uint32_t height) override;

    void RendFrameNV21(uint8_t* y_data, uint32_t y_stride, uint8_t* vu_data, uint32_t vu_stride,
                       uint32_t width, uint32_t height) override;

private:
    void Init();
    void InitContext();
    void DestroyContext();
    void InitShaders();
    void InitTexture();
    void CalcRenderPos();

private:
    uint32_t window_width_{};
    uint32_t window_height_{};
    HDC hdc_{};
    HGLRC hrc_{};

    GLuint texture_y_;
    GLuint texture_u_;
    GLuint texture_v_;
    GLuint uniform_y_;
    GLuint uniform_u_;
    GLuint uniform_v_;
    GLuint shader_program_;
    bool init_{false};
};