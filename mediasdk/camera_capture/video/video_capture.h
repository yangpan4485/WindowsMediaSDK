#pragma once
#include <string>

#include "video_info.h"
#include "video_common.h"
#include "video_event_handler.h"

class VideoCapture {
public:
    VideoCapture();
    virtual ~VideoCapture();
    void SetVideoProfile(const VideoProfile& video_profile);
    void RegisteVideoFrameObserver(std::shared_ptr<IVideoFrameObserver> observer);
    virtual void StartCapture(const std::string& video_device_id);
    virtual void StopCapture();

protected:
    VideoDescription video_desc_{};
    VideoProfile video_profile_{};
    std::weak_ptr<IVideoFrameObserver> frame_observer_{};
};