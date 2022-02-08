#pragma once

#include <initguid.h> 
#include <vector>
#include <dshow.h>
#include <cguid.h>
#include <dvdmedia.h>
#include <unordered_map>

#include "video_common.h"
#include "video/video_info.h"

class VideoDeviceDS {
public:
    static VideoDeviceDS& GetInstance();
    bool Init();

    std::vector<VideoDeviceInfo> GetVideoCaptureDevices();
    std::vector<VideoDescription> GetVideoDeviceFormats(const VideoDeviceInfo& video_device);
    IBaseFilter* GetDeviceFilter(const std::string& device_id);

private:
    VideoDeviceDS();
    ~VideoDeviceDS();

private:
    ICreateDevEnum* ds_dev_enum_{};
    IEnumMoniker* ds_enum_moniker_{};
    bool init_{ false };
    std::unordered_map<std::string, std::vector<VideoDescription>> video_desc_map_{};
};