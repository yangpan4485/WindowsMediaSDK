#pragma once

#include <initguid.h> 
#include <vector>
#include <dshow.h>
#include <cguid.h>
#include <dvdmedia.h>

#include "video/video_capture.h"
#include "sink_filter_ds.h"

class VideoCaptureDS : public VideoCapture {
public:
    VideoCaptureDS();
    ~VideoCaptureDS();

    void StartCapture(const std::string& video_device_id) override;
    void StopCapture() override;
private:
    bool Init();
    HRESULT ConnectDVCamera();
    bool SetCameraOutput(const VideoDescription& video_description);
    bool DisconnectGraph();

private:
    bool initialed_{};
    bool running_{ false };
    std::string video_device_id_{};
    VideoDescription video_description_{};

    IBaseFilter* capture_filter_{};
    IGraphBuilder* graph_builder_{};

    IMediaControl* media_control_{};
    IPin* input_send_pin_{};
    IPin* output_capture_pin_{};

    IBaseFilter* dv_filter_{};
    IPin* input_dv_pin_{};
    IPin* output_dv_pin_{};

    CaptureSinkFilter* sink_filter_{};
    // VideoFrameCallback callback_{};

    uint32_t frame_width_{};
    uint32_t frame_height_{};
};