#pragma once

#include <cstdint>

#include "dxgi_texture.h"
#include "video_frame.h"

class DrawCursor {
public:
    DrawCursor();
    ~DrawCursor();

    void Draw(std::shared_ptr<VideoFrame> video_frame, DXGI_OUTDUPL_POINTER_POSITION& postion,
        DXGI_OUTDUPL_POINTER_SHAPE_INFO& pointer_info, uint8_t* cursor_data);

private:
    uint32_t width_{};
    uint32_t height_{};
};