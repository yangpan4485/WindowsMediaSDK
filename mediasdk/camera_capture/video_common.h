#pragma once

#include <cstdint>
#include <string>

// 插入摄像头并不会影响当前摄像头的使用，拔出摄像头如果还有其它摄像头会自动做切换
enum VideoDeviceEvent {
    kVideoDeviceAdd = 0,
    kVideoDeviceRemove = 1,
};

struct VideoProfile {
    uint32_t width;
    uint32_t height;
    uint32_t fps;
};

struct VideoDeviceInfo {
    std::string device_id;
    std::string device_name;
};