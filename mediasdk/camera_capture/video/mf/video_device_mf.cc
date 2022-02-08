#include "video_device_mf.h"

#include <iostream>
#include <atlbase.h>

#include "string_utils.h"
#include "mf_utils.h"

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "shlwapi.lib")

bool VideoDeviceMF::is_first_ = true;
bool VideoDeviceMF::is_load_ = true;

VideoDeviceMF::VideoDeviceMF() {}

VideoDeviceMF::~VideoDeviceMF() {
    Clear();
    RELEASE_AND_CLEAR(attributes_);
    if (init_) {
        CoUninitialize();
    }
}

bool VideoDeviceMF::LoadMFLibrary() {
    // need check
    if (!is_first_) {
        return is_load_;
    }
    is_first_ = false;
    is_load_ = true;
    return true;
}

VideoDeviceMF& VideoDeviceMF::GetInstance() {
    static VideoDeviceMF instance;
    return instance;
}

std::vector<VideoDeviceInfo> VideoDeviceMF::GetAllVideoDevcies() {
    Clear();
    HRESULT hr = S_OK;
    if (!attributes_) {
        hr = MFCreateAttributes(&attributes_, 1);
        if (FAILED(hr)) {
            return video_devices_;
        }
    }
    hr = attributes_->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr)) {
        return video_devices_;
    }
    hr = MFEnumDeviceSources(attributes_, &imf_active_, &count_);
    if (FAILED(hr)) {
        return video_devices_;
    }

    for (UINT32 i = 0; i < count_; ++i) {
        WCHAR* device_name = NULL;
        WCHAR* device_id = NULL;
        VideoDeviceInfo video_device;
        hr = imf_active_[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &device_name, NULL);
        hr = imf_active_[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &device_id, NULL);
        video_device.device_id = utils::UnicodeToUtf8(device_id);
        video_device.device_name = utils::UnicodeToUtf8(device_name);
        video_devices_.push_back(std::move(video_device));
        CoTaskMemFree(device_name);
        CoTaskMemFree(device_id);
    }
    return video_devices_;
}

std::vector<VideoDescription> VideoDeviceMF::GetVideoFormats(const VideoDeviceInfo& video_device) {
    if (video_desc_map_.find(video_device.device_id) != video_desc_map_.end()) {
        return video_desc_map_[video_device.device_id];
    }
    if (!init_) {
        return std::vector<VideoDescription>();
    }
    IMFMediaSource* source = NULL;
    int index = GetIndex(video_device.device_id);
    HRESULT hr = imf_active_[index]->ActivateObject(__uuidof(IMFMediaSource), (void**)&source);
    if (FAILED(hr)) {
        return std::vector<VideoDescription>();
    }

    CComPtr<IMFPresentationDescriptor> pd = nullptr;
    CComPtr<IMFStreamDescriptor> sd = nullptr;
    CComPtr<IMFMediaTypeHandler> handle = nullptr;

    BOOL bSelected = false;
    unsigned long types = 0;
    hr = source->CreatePresentationDescriptor(&pd);
    if (FAILED(hr)) {
        return std::vector<VideoDescription>();
    }
    hr = pd->GetStreamDescriptorByIndex(0, &bSelected, &sd);
    if (FAILED(hr)) {
        return std::vector<VideoDescription>();
    }
    hr = sd->GetMediaTypeHandler(&handle);
    if (FAILED(hr)) {
        return std::vector<VideoDescription>();
    }
    hr = handle->GetMediaTypeCount(&types);
    if (FAILED(hr)) {
        return std::vector<VideoDescription>();
    }

    for (DWORD i = 0; i < types; i++) {
        CComPtr<IMFMediaType> type = nullptr;
        hr = handle->GetMediaTypeByIndex(i, &type);
        if (FAILED(hr)) {
            continue;
        }
        VideoDescription video_desc;
        UINT32 frameRate = 0u;
        UINT32 denominator = 0u;
        MFGetAttributeRatio(type, MF_MT_FRAME_RATE, &frameRate, &denominator);
        if (frameRate > 30 || frameRate < 10 || (frameRate % 5 != 0)) {
            continue;
        }
        video_desc.fps = frameRate;
        UINT32 width, height;
        MFGetAttributeSize(type, MF_MT_FRAME_SIZE, &width, &height);
        if (width % 10 != 0 || height % 10 != 0 || width < 320 || height < 240 ||
            width > 1280 || height > 720) {
            continue;
        }
        video_desc.width = width;
        video_desc.height = height;
        GUID subtype = { 0 };
        hr = type->GetGUID(MF_MT_SUBTYPE, &subtype);
        video_desc.video_type = GetMFVideoType(subtype);
        if (video_desc.video_type == kVideoTypeUnknown) {
            continue;
        }
        video_desc_map_[video_device.device_id].push_back(std::move(video_desc));
    }
    return video_desc_map_[video_device.device_id];
}

IMFActivate* VideoDeviceMF::GetMFActive(const std::string& device_id) {
    int index = 0;
    for (size_t i = 0; i < video_devices_.size(); ++i) {
        if (video_devices_[i].device_id == device_id) {
            index = i;
            break;
        }
    }
    return imf_active_[index];
}

IMFAttributes* VideoDeviceMF::GetMFAttrutes() {
    return attributes_;
}

bool VideoDeviceMF::Init() {
    if (init_) {
        return true;
    }
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    // HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr)) {
        return false;
    }
    hr = MFStartup(MF_VERSION);
    if (FAILED(hr)) {
        CoUninitialize();
        return false;
    }
    init_ = true;
    return true;
}

void VideoDeviceMF::Clear() {
    for (UINT32 i = 0; i < count_; i++) {
        RELEASE_AND_CLEAR(imf_active_[i]);
    }
    CoTaskMemFree(imf_active_);
    imf_active_ = NULL;
    count_ = 0;
    std::vector<VideoDeviceInfo>().swap(video_devices_);
}

int VideoDeviceMF::GetIndex(const std::string& video_device_id) {
    for (size_t i = 0; i < video_devices_.size(); ++i) {
        if (video_devices_[i].device_id == video_device_id) {
            return i;
        }
    }
    return 0;
}