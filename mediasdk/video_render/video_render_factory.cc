#include "video_render_factory.h"

#include "d3d9/video_render_d3d9.h"
#include "SDL/video_render_sdl.h"
#include "opengl/video_render_opengl.h"

VideoRenderFactory* VideoRenderFactory::CreateInstance() {
    static VideoRenderFactory* instance = new VideoRenderFactory();
    return instance;
}

std::shared_ptr<VideoRender> VideoRenderFactory::CreateVideoRender(RenderType type) {
    switch (type) {
    case kRenderTypeD3D9:
        return std::make_shared<VideoRenderD3d9>();
        break;
    case kRenderTypeSDL:
        return std::make_shared<VideoRenderSDL>();
        break;
    case kRenderTypeOpenGL:
        return std::make_shared<VideoRenderOpenGL>();
        break;
    default:
        break;
    }
    return nullptr;
}

VideoRenderFactory::VideoRenderFactory() {
}

VideoRenderFactory::~VideoRenderFactory() {

}