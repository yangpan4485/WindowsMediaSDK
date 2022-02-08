#pragma once
#include <functional>
#include <fstream>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>

#include "video/video_info.h"
#include "../video_capture.h"

class VideoCaptureMFSourceReader : public VideoCapture, public IMFSourceReaderCallback {
public:
    VideoCaptureMFSourceReader();
    ~VideoCaptureMFSourceReader();

    void StartCapture(const std::string& video_device_id) override;
    void StopCapture() override;

private:
    HRESULT STDMETHODCALLTYPE OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags,
        LONGLONG llTimestamp, IMFSample *pSample) override;
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv) override;
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;
    STDMETHODIMP OnFlush(_In_  DWORD dwStreamIndex) override;
    STDMETHODIMP OnEvent(_In_  DWORD dwStreamIndex, _In_  IMFMediaEvent *pEvent) override;

private:
    IMFSourceReader* source_reader_{};
    long m_nRefCount{};
    CRITICAL_SECTION m_critsec{};
    VideoDescription video_desc_{};
    std::string video_device_id_{};
    bool runing_{};
};