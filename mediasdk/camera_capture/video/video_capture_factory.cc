#include "video_capture_factory.h"

#include "dshow/video_capture_dshow.h"
#include "mf/video_capture_mf_engine.h"
#include "mf/video_capture_mf_source_reader.h"
#include "mf/video_device_mf.h"

VideoCaptureFactory& VideoCaptureFactory::GetInstance() {
    static VideoCaptureFactory instance;
    return instance;
}

std::shared_ptr<VideoCapture> VideoCaptureFactory::CreateVideoCapture() {
     if (VideoDeviceMF::LoadMFLibrary()) {
        // return std::make_shared<VideoCaptureMFSourceReader>();
        return std::make_shared<VideoCaptureMFEngine>();
    }
    return std::make_shared<VideoCaptureDS>();
}

VideoCaptureFactory::VideoCaptureFactory() {}

VideoCaptureFactory::~VideoCaptureFactory() {}