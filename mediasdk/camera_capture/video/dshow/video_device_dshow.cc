#include "video_device_dshow.h"

#include <iostream>

#include "dshow_utils.h"
#include "string_utils.h"

VideoDeviceDS::VideoDeviceDS() {

}

VideoDeviceDS::~VideoDeviceDS() {
    if (ds_enum_moniker_) {
        ds_enum_moniker_ = nullptr;
    }
    if (ds_dev_enum_) {
        ds_dev_enum_ = nullptr;
    }
    // RELEASE_AND_CLEAR(ds_enum_moniker_);
    // RELEASE_AND_CLEAR(ds_dev_enum_);
    if (init_) {
        CoUninitialize();
    }
    init_ = false;
}

VideoDeviceDS& VideoDeviceDS::GetInstance() {
    static VideoDeviceDS instance;
    return instance;
}

std::vector<VideoDeviceInfo> VideoDeviceDS::GetVideoCaptureDevices() {
    std::vector<VideoDeviceInfo> video_infos;
    if (!init_) {
        Init();
    }
    HRESULT hr = ds_dev_enum_->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &ds_enum_moniker_, 0);
    if (FAILED(hr)) {
        return video_infos;
    }
    ds_enum_moniker_->Reset();
    IMoniker* moniker;
    ULONG fetched;
    while (S_OK == ds_enum_moniker_->Next(1, &moniker, &fetched)) {
        IPropertyBag* bag;
        hr = moniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&bag);
        if (FAILED(hr)) {
            continue;
        }
        VARIANT var_name;
        VariantInit(&var_name);
        hr = bag->Read(L"Description", &var_name, 0);
        if (FAILED(hr)) {
            hr = bag->Read(L"FriendlyName", &var_name, 0);
        }
        if (SUCCEEDED(hr)) {
            // ignore all VFW drivers
            if ((wcsstr(var_name.bstrVal, (L"(VFW)")) == NULL) &&
                (_wcsnicmp(var_name.bstrVal, (L"Google Camera Adapter"), 21) != 0)) {
                std::string name = utils::UnicodeToUtf8(var_name.bstrVal);
                std::string id = name;
                hr = bag->Read(L"DevicePath", &var_name, 0);
                if (SUCCEEDED(hr)) {
                    id = utils::UnicodeToUtf8(var_name.bstrVal);
                }
                VideoDeviceInfo video_info{ id, name };
                video_infos.push_back(std::move(video_info));
            }

        }
        VariantClear(&var_name);
        bag->Release();
        moniker->Release();
    }
    return video_infos;
}

std::vector<VideoDescription> VideoDeviceDS::GetVideoDeviceFormats(const VideoDeviceInfo& video_device) {
    if (video_desc_map_.find(video_device.device_id) != video_desc_map_.end()) {
        return video_desc_map_[video_device.device_id];
    }
    IBaseFilter* capture_device = nullptr;
    IPin* output_capture_pin = nullptr;
    IAMExtDevice* ext_device = nullptr;
    IAMStreamConfig* stream_config = nullptr;
    IAMVideoControl* video_control_config = nullptr;
    AM_MEDIA_TYPE* pmt = nullptr;
    std::vector<VideoDescription> video_description_vec{};
    std::shared_ptr<void> raii(nullptr, [&](void*) {
        RELEASE_AND_CLEAR(capture_device);
        RELEASE_AND_CLEAR(output_capture_pin);
        RELEASE_AND_CLEAR(ext_device);
        RELEASE_AND_CLEAR(stream_config);
        RELEASE_AND_CLEAR(video_control_config);
        });

    capture_device = GetDeviceFilter(video_device.device_id);
    if (!capture_device) {
        return video_description_vec;
    }

    output_capture_pin = GetOutputPin(capture_device, GUID_NULL);
    if (!output_capture_pin) {
        return video_description_vec;
    }
    HRESULT hr = capture_device->QueryInterface(IID_IAMExtDevice, (void**)&ext_device);
    if (SUCCEEDED(hr) && ext_device) {
        ext_device->Release();
    }

    hr = output_capture_pin->QueryInterface(IID_IAMStreamConfig, (void**)&stream_config);
    if (FAILED(hr)) {
        return video_description_vec;
    }

    // this gets the FPS
    HRESULT hr_vc = capture_device->QueryInterface(IID_IAMVideoControl, (void**)&video_control_config);

    int count, size;
    hr = stream_config->GetNumberOfCapabilities(&count, &size);
    if (FAILED(hr)) {
        return video_description_vec;
    }

    bool supportFORMAT_VideoInfo2 = false;
    bool supportFORMAT_VideoInfo = false;
    bool foundInterlacedFormat = false;
    GUID preferedVideoDescription = FORMAT_VideoInfo;
    VIDEO_STREAM_CONFIG_CAPS caps;

    for (int32_t tmp = 0; tmp < count; ++tmp) {
        hr = stream_config->GetStreamCaps(tmp, &pmt, reinterpret_cast<BYTE*>(&caps));
        if (hr == S_OK) {
            if (pmt->majortype == MEDIATYPE_Video && pmt->formattype == FORMAT_VideoInfo2) {
                supportFORMAT_VideoInfo2 = true;
                VIDEOINFOHEADER2* h = reinterpret_cast<VIDEOINFOHEADER2*>(pmt->pbFormat);
                foundInterlacedFormat |= h->dwInterlaceFlags & (AMINTERLACE_IsInterlaced | AMINTERLACE_DisplayModeBobOnly);
            }
            if (pmt->majortype == MEDIATYPE_Video &&
                pmt->formattype == FORMAT_VideoInfo) {
                supportFORMAT_VideoInfo = true;
            }
        }
    }

    if (supportFORMAT_VideoInfo2) {
        if (supportFORMAT_VideoInfo && !foundInterlacedFormat) {
            preferedVideoDescription = FORMAT_VideoInfo;
        }
        else {
            preferedVideoDescription = FORMAT_VideoInfo2;
        }
    }

    for (int32_t tmp = 0; tmp < count; ++tmp) {
        hr = stream_config->GetStreamCaps(tmp, &pmt, reinterpret_cast<BYTE*>(&caps));
        if (pmt->majortype == MEDIATYPE_Video && pmt->formattype == preferedVideoDescription) {
            VideoDescription video_desc;
            int64_t avgTimePerFrame = 0;
            if (pmt->formattype == FORMAT_VideoInfo) {
                VIDEOINFOHEADER* h = reinterpret_cast<VIDEOINFOHEADER*>(pmt->pbFormat);
                video_desc.width = h->bmiHeader.biWidth;
                video_desc.height = h->bmiHeader.biHeight;
                avgTimePerFrame = h->AvgTimePerFrame;
            }
            if (pmt->formattype == FORMAT_VideoInfo2) {
                VIDEOINFOHEADER2* h = reinterpret_cast<VIDEOINFOHEADER2*>(pmt->pbFormat);
                video_desc.width = h->bmiHeader.biWidth;
                video_desc.height = h->bmiHeader.biHeight;
                avgTimePerFrame = h->AvgTimePerFrame;
            }

            if (hr_vc == S_OK) {
                LONGLONG* frameDurationList;
                long listSize;
                SIZE size;
                size.cx = video_desc.width;
                size.cy = video_desc.height;
                hr_vc = video_control_config->GetFrameRateList(output_capture_pin, tmp, size, &listSize, &frameDurationList);
                LONGLONG maxFPS = GetMaxOfFrameArray(frameDurationList, listSize);
                if (hr_vc == S_OK && listSize > 0 && maxFPS != 0) {
                    video_desc.fps = static_cast<int>(10000000 / maxFPS);
                }
                else {
                    if (avgTimePerFrame > 0) {
                        video_desc.fps = static_cast<int>(10000000 / avgTimePerFrame);
                    }
                    else {
                        video_desc.fps = 0;
                    }
                }
            }
            else {
                if (avgTimePerFrame > 0) {
                    video_desc.fps = static_cast<int>(10000000 / avgTimePerFrame);
                }
                else {
                    video_desc.fps = 0;
                }
            }
            video_desc.video_type = GetVideoType(pmt->subtype);
            video_description_vec.push_back(std::move(video_desc));

        }
        FreeMediaType(pmt);
        pmt = NULL;
    }
    return video_description_vec;
}

bool VideoDeviceDS::Init() {
    if (init_) {
        return true;
    }
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        return false;
    }

    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void**)&ds_dev_enum_);
    if (FAILED(hr)) {
        CoUninitialize();
        return false;
    }
    init_ = true;
    return true;
}

IBaseFilter* VideoDeviceDS::GetDeviceFilter(const std::string& device_id) {
    if (device_id.length() > kMaxDeviceLength) {
        return nullptr;
    }
    if (!init_) {
        Init();
    }

    RELEASE_AND_CLEAR(ds_enum_moniker_);
    HRESULT hr = ds_dev_enum_->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &ds_enum_moniker_, 0);
    if (hr != NOERROR) {
        return nullptr;
    }
    ds_enum_moniker_->Reset();
    ULONG fetched;
    IMoniker* moniker;
    IBaseFilter* capture_filter = NULL;
    bool device_found = false;
    while (S_OK == ds_enum_moniker_->Next(1, &moniker, &fetched) && !device_found) {
        IPropertyBag* bag;
        hr = moniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&bag);
        if (hr == S_OK) {
            VARIANT var_name;
            VariantInit(&var_name);
            hr = bag->Read(L"DevicePath", &var_name, 0);
            if (FAILED(hr)) {
                hr = bag->Read(L"Description", &var_name, 0);
                if (FAILED(hr)) {
                    hr = bag->Read(L"FriendlyName", &var_name, 0);
                }
            }
            if (SUCCEEDED(hr)) {
                char temp[256] = "";
                WideCharToMultiByte(CP_UTF8, 0, var_name.bstrVal, -1, temp, sizeof(temp), NULL, NULL);
                if (device_id == temp) {
                    device_found = true;
                    hr = moniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&capture_filter);
                }
            }
            VariantClear(&var_name);
            bag->Release();
        }

        moniker->Release();
    }
    return capture_filter;
}