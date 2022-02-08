#pragma once

#include "video_render.h"
#include "SDL2/SDL.h"

class VideoRenderSDL : public VideoRender {
public:
    VideoRenderSDL();
    ~VideoRenderSDL();

    void RendFrame(uint8_t* data, uint32_t width, uint32_t height) override;

    void RendFrameI420(uint8_t* y_data, uint32_t y_stride, uint8_t* u_data,
        uint32_t u_stride, uint8_t* v_data, uint32_t v_stride,
        uint32_t width, uint32_t height) override;

    void RendFrameNV12(uint8_t* y_data, uint32_t y_stride, uint8_t* uv_data,
        uint32_t uv_stride, uint32_t width, uint32_t height) override;

    void RendFrameNV21(uint8_t* y_data, uint32_t y_stride, uint8_t* vu_data,
        uint32_t vu_stride, uint32_t width, uint32_t height) override;

private:
    void InitSDL();
    void Destroy();
    void InitRender();
    void InitTexture();
    void Update(uint32_t width, uint32_t height, PixFormat pix_format);

private:
    SDL_Window* window_{};
    SDL_Renderer* renderer_{};
    SDL_Texture* texture_{};
};