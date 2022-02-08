#pragma once
#include "screen_common.h"
#include "video_frame.h"

class ICaptureHandler {
public:
    ICaptureHandler() {}
    virtual ~ICaptureHandler() {}

    // 回调视频帧
    virtual void OnScreenFrame(std::shared_ptr<VideoFrame> video_frame) = 0;

    // 当窗口关闭时回调
    virtual void OnScreenEvent(ScreenEvent screen_event) = 0;
};