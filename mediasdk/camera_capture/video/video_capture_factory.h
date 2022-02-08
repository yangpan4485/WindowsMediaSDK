#pragma once

#include "video_capture.h"

class VideoCaptureFactory {
public:
    static VideoCaptureFactory& GetInstance();

    std::shared_ptr<VideoCapture> CreateVideoCapture();

private:
    VideoCaptureFactory();
    ~VideoCaptureFactory();
    VideoCaptureFactory(const VideoCaptureFactory&) = delete;
    VideoCaptureFactory operator=(const VideoCaptureFactory&) = delete;
};