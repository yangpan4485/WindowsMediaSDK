#pragma once

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <vector>
#include <unordered_map>

#include "video_common.h"
#include "video/video_info.h"

class VideoDeviceMF {
public:
    static bool LoadMFLibrary();
    static VideoDeviceMF& GetInstance();
    bool Init();

    std::vector<VideoDeviceInfo> GetAllVideoDevcies();
    std::vector<VideoDescription> GetVideoFormats(const VideoDeviceInfo& video_device);

    IMFActivate* GetMFActive(const std::string& device_id);
    IMFAttributes* GetMFAttrutes();

private:
    VideoDeviceMF();
    ~VideoDeviceMF();
    VideoDeviceMF(const VideoDeviceMF&) = delete;
    VideoDeviceMF operator=(const VideoDeviceMF&) = delete;

    void Clear();
    int GetIndex(const std::string& video_device_id);

private:
    IMFAttributes* attributes_{};
    IMFActivate** imf_active_{};
    UINT32 count_{};
    bool init_;
    std::vector<VideoDeviceInfo> video_devices_{};
    std::unordered_map<std::string, std::vector<VideoDescription>> video_desc_map_{};
    static bool is_first_;
    static bool is_load_;
};