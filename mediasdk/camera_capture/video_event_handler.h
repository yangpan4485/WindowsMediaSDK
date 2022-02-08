#pragma once
#include <string>

#include "video_common.h"
#include "video_frame.h"

class IVideoDeviceEventHandler {
public:
    IVideoDeviceEventHandler() {}
    virtual ~IVideoDeviceEventHandler() {}

    // 摄像头插拔回调
    virtual void OnVideoDeviceEvent(VideoDeviceEvent device_event,
                                    const std::string& device_id) = 0;
};

class IVideoFrameObserver {
public:
    IVideoFrameObserver() {}
    virtual ~IVideoFrameObserver() {}

    // 返回摄像头打开失败的错误码
    virtual void OnVideoError(int error_code, const std::string& device_name) = 0;

    // 采集数据帧回调
    virtual void OnVideoFrame(std::shared_ptr<VideoFrame> video_frame) = 0;
};