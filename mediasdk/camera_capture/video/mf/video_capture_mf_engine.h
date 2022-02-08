#pragma once
#include <mfcaptureengine.h>
#include <strsafe.h>
#include <commctrl.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "../video_capture.h"
#include "video/video_info.h"
#include "video_common.h"

class MFVideoCallback;

class VideoCaptureMFEngine : public VideoCapture {
public:
    VideoCaptureMFEngine();
    ~VideoCaptureMFEngine();

    void StartCapture(const std::string& video_device_id) override;
    void StopCapture() override;

    void OnEvent(IMFMediaEvent* media_event);
    void OnFrame(uint8_t* data, int length);
private:
    bool InitCaptureEngine(const VideoDeviceInfo& video_device);
    bool CreateD3DManager();
    bool GetAvailableIndex(IMFCaptureSource* source, int& stream_index, int& media_type_index, const VideoDescription& video_description);
    HRESULT WaitOnCaptureEvent(GUID capture_event_guid);

private:
    IMFCaptureEngine* capture_engine_{};
    IMFCaptureSource* capture_source_{};
    // Microsoft::WRL::ComPtr<IMFCaptureEngine> capture_engine_{};
    Microsoft::WRL::ComPtr<IMFCaptureEngineClassFactory> capture_engine_class_factory_{};
    MFVideoCallback* video_callback_{};

    bool is_initialized_{};
    bool is_started_{};

    HANDLE error_handle_{};
    HANDLE initial_handle_{};

    Microsoft::WRL::ComPtr<IMFMediaSource> source_{};
    Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> dxgi_device_manager_{};
    Microsoft::WRL::ComPtr<ID3D11Device> dx11_device_{};
    UINT reset_token_{};
    std::string video_device_id_{};
};
