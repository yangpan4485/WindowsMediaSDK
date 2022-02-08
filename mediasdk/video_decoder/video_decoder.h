#pragma once
#include <Windows.h>
#include <cstdint>
#include <functional>
#include "video_frame.h"

class VideoDecoder {
    using DevoceFrameCallback = std::function<void(const std::shared_ptr<VideoFrame>& video_frame)>;
public:
    VideoDecoder();
    virtual ~VideoDecoder();

    virtual void Decode(uint8_t* data, uint32_t len);
    virtual void SetRenderHwnd(HWND hwnd);
    void SetDevoceFrameCallback(DevoceFrameCallback callback);

protected:
    DevoceFrameCallback callback_{};
};