#include "video_capture_dshow.h"

#include <iostream>
#include <memory>

#include "dshow_utils.h"
#include "libyuv.h"
#include "video/video_utils.h"
#include "video_device_dshow.h"
#include "video_frame.h"

#define CAPTURE_FILTER_NAME L"VideoCaptureFilter"
#define SINK_FILTER_NAME L"SinkFilter"

VideoCaptureDS::VideoCaptureDS() {}

VideoCaptureDS::~VideoCaptureDS() {
    if (media_control_) {
        media_control_->Stop();
    }
    if (graph_builder_) {
        if (sink_filter_) {
            graph_builder_->RemoveFilter(sink_filter_);
        }
        if (capture_filter_) {
            graph_builder_->RemoveFilter(capture_filter_);
        }
        if (dv_filter_) {
            graph_builder_->RemoveFilter(dv_filter_);
        }
    }

    RELEASE_AND_CLEAR(input_send_pin_);
    RELEASE_AND_CLEAR(output_capture_pin_);
    RELEASE_AND_CLEAR(capture_filter_); // release the capture device
    RELEASE_AND_CLEAR(dv_filter_);
    RELEASE_AND_CLEAR(media_control_);
    RELEASE_AND_CLEAR(input_dv_pin_);
    RELEASE_AND_CLEAR(output_dv_pin_);
    RELEASE_AND_CLEAR(graph_builder_);
}

void VideoCaptureDS::StartCapture(const std::string& video_device_id) {
    if (running_) {
        return;
    }
    auto devices = VideoDeviceDS::GetInstance().GetVideoCaptureDevices();
    for (size_t i = 0; i < devices.size(); ++i) {
        size_t pos = devices[i].device_id.find("{");
        size_t video_pos = video_device_id.find("{");
        if (devices[i].device_id.substr(0, pos) == video_device_id.substr(0, video_pos)) {
            video_device_id_ = devices[i].device_id;
        }
    }
    if (video_device_id_.empty()) {
        return;
    }

    VideoDeviceInfo video_device_info;
    video_device_info.device_id = video_device_id;
    std::vector<VideoDescription> description =
        VideoDeviceDS::GetInstance().GetVideoDeviceFormats(video_device_info);
    video_description_ = FindVideoDescription(description, video_profile_);
    running_ = true;
    if (!Init()) {
        return;
    }
    if (!DisconnectGraph()) {
        return;
    }
    if (!SetCameraOutput(video_description_)) {
        return;
    }
    HRESULT hr = media_control_->Run();
    if (FAILED(hr)) {
        return;
    }
    running_ = true;
}

void VideoCaptureDS::StopCapture() {
    if (!running_) {
        return;
    }
    if (media_control_) {
        // media_control_->Pause();
        media_control_->Stop();
    }
    running_ = false;
}

bool VideoCaptureDS::Init() {
    capture_filter_ = VideoDeviceDS::GetInstance().GetDeviceFilter(video_device_id_);
    if (!capture_filter_) {
        std::cout << "Failed to get device filter" << std::endl;
        return false;
    }

    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder,
                                  (void**)&graph_builder_);
    if (FAILED(hr)) {
        std::cout << "Failed to create graph builder" << std::endl;
        return false;
    }

    hr = graph_builder_->QueryInterface(IID_IMediaControl, (void**)&media_control_);
    if (FAILED(hr)) {
        return false;
    }

    hr = graph_builder_->AddFilter(capture_filter_, CAPTURE_FILTER_NAME);
    if (FAILED(hr)) {
        return false;
    }

    output_capture_pin_ = GetOutputPin(capture_filter_, PIN_CATEGORY_CAPTURE);
    if (FAILED(hr)) {
        return false;
    }

    sink_filter_ = new CaptureSinkFilter();
    sink_filter_->RegisterCallback([&, this](unsigned char* buffer, size_t length,
                                             VideoDescription video_description) {
        const int32_t width = video_description.width;
        const int32_t height = video_description.height;
        std::shared_ptr<VideoFrame> video_frame;
        video_frame.reset(new VideoFrame(width, height, kFrameTypeI420, true));
        uint8_t* data = video_frame->GetData();
        uint32_t y_linesize = video_frame->GetPitch();
        uint32_t u_linesize = y_linesize / 2;
        uint32_t v_linesize = y_linesize / 2;
        int result = libyuv::ConvertToI420(buffer, length, data, y_linesize, data + width * height,
                                           u_linesize, data + width * height * 5 / 4, v_linesize, 0,
                                           0, // No Cropping
                                           width, height, width, height, libyuv::kRotate0,
                                           ConvertVideoType(video_description.video_type));
        if (result < 0) {
            return;
        }
        auto frame_observer = frame_observer_.lock();
        if (frame_observer) {
            frame_observer->OnVideoFrame(video_frame);
        }
    });
    hr = graph_builder_->AddFilter(sink_filter_, SINK_FILTER_NAME);
    if (FAILED(hr)) {
        return false;
    }

    input_send_pin_ = GetInputPin(sink_filter_);
    if (!input_send_pin_) {
        return false;
    }

    hr = media_control_->Pause();
    if (FAILED(hr)) {
        return false;
    }
    return true;
}

HRESULT VideoCaptureDS::ConnectDVCamera() {
    HRESULT hr = S_OK;
    if (!dv_filter_) {
        hr = CoCreateInstance(CLSID_DVVideoCodec, NULL, CLSCTX_INPROC, IID_IBaseFilter,
                              (void**)&dv_filter_);
        if (hr != S_OK) {
            return hr;
        }
        hr = graph_builder_->AddFilter(dv_filter_, L"VideoDecoderDV");
        if (hr != S_OK) {
            return hr;
        }
        input_dv_pin_ = GetInputPin(dv_filter_);
        if (input_dv_pin_ == NULL) {
            return -1;
        }
        output_dv_pin_ = GetOutputPin(dv_filter_, GUID_NULL);
        if (output_dv_pin_ == NULL) {
            return -1;
        }
    }
    hr = graph_builder_->ConnectDirect(output_capture_pin_, input_dv_pin_, NULL);
    if (hr != S_OK) {
        return hr;
    }

    hr = graph_builder_->ConnectDirect(output_dv_pin_, input_send_pin_, NULL);
    if (hr != S_OK) {
        std::cout << "Failed connect direct" << std::endl;
    }
    return hr;
}

bool VideoCaptureDS::SetCameraOutput(const VideoDescription& video_description) {
    IAMStreamConfig* stream_config = NULL;
    AM_MEDIA_TYPE* pmt = NULL;
    VIDEO_STREAM_CONFIG_CAPS caps;

    HRESULT hr = output_capture_pin_->QueryInterface(IID_IAMStreamConfig, (void**)&stream_config);
    if (hr) {
        return false;
    }
    bool is_dv_camera = false;
    hr = stream_config->GetStreamCaps(0, &pmt, reinterpret_cast<BYTE*>(&caps));
    if (hr == S_OK) {
        /*if (pmt->formattype == FORMAT_VideoInfo2) {
            VIDEOINFOHEADER2* h = reinterpret_cast<VIDEOINFOHEADER2*>(pmt->pbFormat);
            if (video_description.fps > 0 && extern_description.frame_control) {
                h->AvgTimePerFrame = REFERENCE_TIME(10000000.0 / video_description.fps);
            }
        }
        else {
            VIDEOINFOHEADER* h = reinterpret_cast<VIDEOINFOHEADER*>(pmt->pbFormat);
            if (video_description.fps > 0 && extern_description.frame_control) {
                h->AvgTimePerFrame = REFERENCE_TIME(10000000.0 / video_description.fps);
            }
        }*/
        sink_filter_->SetVideoDescription(video_description);
        hr += stream_config->SetFormat(pmt);
        if (pmt->subtype == MEDIASUBTYPE_dvsl || pmt->subtype == MEDIASUBTYPE_dvsd ||
            pmt->subtype == MEDIASUBTYPE_dvhd) {
            is_dv_camera = true; // This is a DV camera. Use MS DV filter
        }
    }

    RELEASE_AND_CLEAR(stream_config);
    if (FAILED(hr)) {
        return false;
    }
    if (is_dv_camera) {
        hr = ConnectDVCamera();
    } else {
        hr = graph_builder_->ConnectDirect(output_capture_pin_, input_send_pin_, NULL);
    }
    if (hr != S_OK) {
        return false;
    }
    return true;
}

bool VideoCaptureDS::DisconnectGraph() {
    HRESULT hr = media_control_->Stop();
    hr += graph_builder_->Disconnect(output_capture_pin_);
    hr += graph_builder_->Disconnect(input_send_pin_);
    // if the DV camera filter exist
    if (dv_filter_) {
        graph_builder_->Disconnect(input_dv_pin_);
        graph_builder_->Disconnect(output_dv_pin_);
    }
    if (hr != S_OK) {
        std::cout << "hr:" << hr << std::endl;
        return true;
    }
    return true;
}