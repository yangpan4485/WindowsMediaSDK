#include "video_device_manager.h"

#include "dshow/video_device_dshow.h"
#include "mf/video_device_mf.h"

std::vector<VideoDeviceInfo> VideoDeviceManager::EnumVideoDevices() {
    if (VideoDeviceMF::LoadMFLibrary() && VideoDeviceMF::GetInstance().Init()) {
        return VideoDeviceMF::GetInstance().GetAllVideoDevcies();
    }
    if (!VideoDeviceDS::GetInstance().Init()) {
        return std::vector<VideoDeviceInfo>();
    }
    return VideoDeviceDS::GetInstance().GetVideoCaptureDevices();
}

VideoDeviceManager::VideoDeviceManager() {

}

VideoDeviceManager::~VideoDeviceManager() {

}

void VideoDeviceManager::SetVideoDeviceChangeCallback(VideoDeviceChangeCallback callback) {
    video_device_monitor_.SetVideoDeviceChangeCallback(callback);
}

VideoDeviceInfo VideoDeviceManager::GetDefaultVideoDevice() {
    return VideoDeviceInfo();
}

std::string VideoDeviceManager::GetVideoDeviceId(const std::string& video_device_name) {
    return "";
}