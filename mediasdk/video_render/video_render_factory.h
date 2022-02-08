#pragma once
#include <memory>

#include "video_render.h"

enum RenderType {
    kRenderTypeD3D9,
    kRenderTypeSDL,
    kRenderTypeOpenGL
};

class VideoRenderFactory {
public:
    static VideoRenderFactory* CreateInstance();
    
    std::shared_ptr<VideoRender> CreateVideoRender(RenderType type);
private:
    VideoRenderFactory();
    ~VideoRenderFactory();
    VideoRenderFactory(const VideoRenderFactory&) = delete;
    VideoRenderFactory operator=(const VideoRenderFactory&) = delete;
};