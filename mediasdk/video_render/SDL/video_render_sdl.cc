#include "video_render_sdl.h"

#include <iostream>

VideoRenderSDL::VideoRenderSDL() {
    InitSDL();
}

VideoRenderSDL::~VideoRenderSDL() {
    Destroy();
}

void VideoRenderSDL::RendFrame(uint8_t* data, uint32_t width, uint32_t height) {
    Update(width, height, I420);
    SDL_UpdateTexture(texture_, NULL, data, frame_width_);
    RECT rect;
    GetClientRect(render_window_, &rect);

    SDL_Rect sdl_rect;
    sdl_rect.x = rect.left;
    sdl_rect.y = rect.top;
    sdl_rect.w = rect.right - rect.left;
    sdl_rect.h = rect.bottom - rect.top;
    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, texture_, NULL, &sdl_rect);
    SDL_RenderPresent(renderer_);
}

void VideoRenderSDL::RendFrameI420(uint8_t* y_data, uint32_t y_stride, uint8_t* u_data,
    uint32_t u_stride, uint8_t* v_data, uint32_t v_stride,
    uint32_t width, uint32_t height) {
    Update(width, height, I420);
    SDL_UpdateYUVTexture(texture_, NULL, y_data, y_stride, u_data, u_stride, v_data, v_stride);
    RECT rect;
    GetClientRect(render_window_, &rect);

    SDL_Rect sdl_rect;
    sdl_rect.x = rect.left;
    sdl_rect.y = rect.top;
    sdl_rect.w = rect.right - rect.left;
    sdl_rect.h = rect.bottom - rect.top;
    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, texture_, NULL, &sdl_rect);
    SDL_RenderPresent(renderer_);
}

void VideoRenderSDL::RendFrameNV12(uint8_t* y_data, uint32_t y_stride, uint8_t* uv_data,
    uint32_t uv_stride, uint32_t width, uint32_t height) {
    // 暂不支持
    return;
}

void VideoRenderSDL::RendFrameNV21(uint8_t* y_data, uint32_t y_stride, uint8_t* vu_data,
    uint32_t vu_stride, uint32_t width, uint32_t height) {
    // 暂不支持
}

void VideoRenderSDL::InitSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1) {
        std::cout << "SDL2 init failed" << std::endl;
        return;
    }
    std::cout << "init sdl" << std::endl;
}

void VideoRenderSDL::Destroy() {
    if (window_) {
        SDL_DestroyWindow(window_);
    }
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
    }
    if (texture_) {
        SDL_DestroyTexture(texture_);
    }
    SDL_Quit();
}

void VideoRenderSDL::InitRender() {
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
    }
    std::cout << "SDL_CreateRenderer" << std::endl;
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
}

void VideoRenderSDL::InitTexture() {
    uint32_t pixformat;
    if (pix_format_ == I420) {
        pixformat = SDL_PIXELFORMAT_IYUV;
    }
    else if (pix_format_ == NV12) {
        pixformat = SDL_PIXELFORMAT_NV12;
    }
    else if (pix_format_ == NV21) {
        pixformat = SDL_PIXELFORMAT_NV21;
    }
    else {
        std::cout << "输入的格式暂不支持" << std::endl;
    }
    if (texture_) {
        SDL_DestroyTexture(texture_);
    }
    std::cout << "pixformat:" << pixformat << std::endl;
    texture_ = SDL_CreateTexture(renderer_, pixformat, SDL_TEXTUREACCESS_STREAMING, frame_width_, frame_height_);
    std::cout << "init texture" << std::endl;
}

void VideoRenderSDL::Update(uint32_t width, uint32_t height, PixFormat pix_format) {
    if (CheckWindowSizeChange()) {
        if (window_) {
            SDL_DestroyWindow(window_);
        }
        std::cout << "create window" << std::endl;
        window_ = SDL_CreateWindowFrom(render_window_);
    }
    if (frame_width_ != width || frame_height_ != height || pix_format_ != pix_format) {
        frame_width_ = width;
        frame_height_ = height;
        pix_format_ = pix_format;
        InitRender();
        InitTexture();
    }
}