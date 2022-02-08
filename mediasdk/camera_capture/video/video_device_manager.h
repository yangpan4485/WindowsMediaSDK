#pragma once

#include <string>
#include <vector>
#include <memory>

#include "video_event_handler.h"
#include "video_common.h"
#include "video_device_monitor.h"

class VideoDeviceManager {
public:
    std::vector<VideoDeviceInfo> EnumVideoDevices();

    VideoDeviceManager();
    ~VideoDeviceManager();

    void SetVideoDeviceChangeCallback(VideoDeviceChangeCallback callback);

    VideoDeviceInfo GetDefaultVideoDevice();
    std::string GetVideoDeviceId(const std::string& video_device_name);

private:
    VideoDeviceMonitor video_device_monitor_{};
};