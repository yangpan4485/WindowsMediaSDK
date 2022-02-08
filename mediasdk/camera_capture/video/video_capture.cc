#include "video_capture.h"

VideoCapture::VideoCapture() {}

VideoCapture::~VideoCapture() {}

void VideoCapture::SetVideoProfile(const VideoProfile& video_profile) {
    video_profile_ = video_profile;
}

void VideoCapture::RegisteVideoFrameObserver(std::shared_ptr<IVideoFrameObserver> observer) {
    frame_observer_ = observer;
}

void VideoCapture::StartCapture(const std::string& video_device_id) {}

void VideoCapture::StopCapture() {}