#include "video_capture_mf_source_reader.h"

#include <Shlwapi.h>
#include <iostream>

#include "mf_utils.h"
#include "video_device_mf.h"
#include "video/video_utils.h"
#include "libyuv.h"

VideoCaptureMFSourceReader::VideoCaptureMFSourceReader() {
    InitializeCriticalSection(&m_critsec);
}

VideoCaptureMFSourceReader::~VideoCaptureMFSourceReader() {
    // StopCapture();
    DeleteCriticalSection(&m_critsec);
}

void VideoCaptureMFSourceReader::StartCapture(const std::string& video_device_id) {
    EnterCriticalSection(&m_critsec);
    auto devices = VideoDeviceMF::GetInstance().GetAllVideoDevcies();
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
    video_device_info.device_id = video_device_id_;
    auto video_formats = VideoDeviceMF::GetInstance().GetVideoFormats(video_device_info);
    video_desc_ = FindVideoDescription(video_formats, video_profile_);
    IMFAttributes* pAttributes = NULL;
    HRESULT hr = MFCreateAttributes(&pAttributes, 2);
    hr = pAttributes->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, TRUE);
    hr = pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, (IUnknown*)this);
    IMFMediaSource* source = NULL;
    auto active = VideoDeviceMF::GetInstance().GetMFActive(video_device_id_);
    hr = active->ActivateObject(__uuidof(IMFMediaSource), (void**)&source);
    hr = MFCreateSourceReaderFromMediaSource(source, pAttributes, &source_reader_);
    IMFMediaType* pType = NULL;
    if (SUCCEEDED(hr)) {
        for (DWORD i = 0;; i++) {
            hr = source_reader_->GetNativeMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, i,
                                                    &pType);
            GUID guid = GetMFGuidByFormat(video_desc_.video_type);
            pType->SetGUID(MF_MT_SUBTYPE, guid);
            MFSetAttributeSize(pType, MF_MT_FRAME_SIZE, video_desc_.width, video_desc_.height);
            MFSetAttributeRatio(pType, MF_MT_FRAME_RATE, video_desc_.fps, 1);
            hr = source_reader_->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                                                     NULL, pType);
            if (SUCCEEDED(hr)) {
                RELEASE_AND_CLEAR(pType);
                break;
            }
        }
    }

    if (SUCCEEDED(hr)) {
        // Ask for the first sample.
        hr = source_reader_->ReadSample((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, NULL, NULL,
                                        NULL, NULL);
    }

    if (FAILED(hr)) {
        if (source) {
            source->Shutdown();
        }
        StopCapture();
    }

    RELEASE_AND_CLEAR(source);
    RELEASE_AND_CLEAR(pAttributes);
    RELEASE_AND_CLEAR(pType);
    LeaveCriticalSection(&m_critsec);
}

void VideoCaptureMFSourceReader::StopCapture() {
    EnterCriticalSection(&m_critsec);
    if (source_reader_) {
        source_reader_ = nullptr;
    }
    VideoDeviceMF::GetInstance().GetMFActive(video_device_id_)->ShutdownObject();
    LeaveCriticalSection(&m_critsec);
}

HRESULT VideoCaptureMFSourceReader::OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex,
                                                 DWORD dwStreamFlags, LONGLONG llTimestamp,
                                                 IMFSample* pSample) {
    EnterCriticalSection(&m_critsec);
    HRESULT hr = S_OK;
    IMFMediaBuffer* pBuffer = NULL;
    if (FAILED(hrStatus)) {
        hr = hrStatus;
    }
    if (SUCCEEDED(hr)) {
        if (pSample) {
            hr = pSample->GetBufferByIndex(0, &pBuffer);
            if (SUCCEEDED(hr)) {
                BYTE* pData = NULL;
                DWORD size;
                pBuffer->GetCurrentLength(&size);
                hr = pBuffer->Lock(&pData, NULL, NULL);
                const int32_t width = video_desc_.width;
                const int32_t height = video_desc_.height;
                std::shared_ptr<VideoFrame> video_frame;
                video_frame.reset(new VideoFrame(width, height, kFrameTypeI420, true));
                uint8_t* yuv_data = video_frame->GetData();
                uint32_t y_linesize = video_frame->GetPitch();
                uint32_t u_linesize = y_linesize / 2;
                uint32_t v_linesize = y_linesize / 2;
                int result = libyuv::ConvertToI420(
                    pData, size, yuv_data, video_frame->GetPitch(), yuv_data + width * height, u_linesize,
                    yuv_data + width * height * 5 / 4, v_linesize, 0, 0, // No Cropping
                    width, height, width, height, libyuv::kRotate0, ConvertVideoType(video_desc_.video_type));
                if (result < 0) {
                    return S_OK;
                }
                auto frame_observer = frame_observer_.lock();
                if (frame_observer) {
                    frame_observer->OnVideoFrame(video_frame);
                }
                pBuffer->Unlock();
                pBuffer->Release();
                std::cout << "capture success" << std::endl;
            }
        }
    }
    // Request the next frame.
    if (SUCCEEDED(hr)) {
        hr = source_reader_->ReadSample((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, NULL, NULL,
                                        NULL, NULL);
    }
    RELEASE_AND_CLEAR(pBuffer);
    LeaveCriticalSection(&m_critsec);
    return hr;
}

STDMETHODIMP VideoCaptureMFSourceReader::QueryInterface(REFIID iid, void** ppv) {
    static const QITAB qit[] = {
        QITABENT(VideoCaptureMFSourceReader, IMFSourceReaderCallback),
        {0},
    };
    return QISearch(this, qit, iid, ppv);
}

STDMETHODIMP_(ULONG) VideoCaptureMFSourceReader::AddRef() {
    return InterlockedIncrement(&m_nRefCount);
}

STDMETHODIMP_(ULONG) VideoCaptureMFSourceReader::Release() {
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0) {
        delete this;
    }
    return uCount;
}

STDMETHODIMP VideoCaptureMFSourceReader::OnFlush(_In_ DWORD dwStreamIndex) {
    return S_OK;
}

STDMETHODIMP VideoCaptureMFSourceReader::OnEvent(_In_ DWORD dwStreamIndex,
                                                 _In_ IMFMediaEvent* pEvent) {
    return S_OK;
}