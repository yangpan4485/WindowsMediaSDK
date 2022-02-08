#include "dshow_utils.h"

// #include "libyuv.h"

IPin* GetOutputPin(IBaseFilter* filter, REFGUID category) {
    IEnumPins* enum_pin = nullptr;
    HRESULT hr = filter->EnumPins(&enum_pin);
    if (hr != S_OK || !enum_pin) {
        return nullptr;
    }
    enum_pin->Reset();
    IPin* pin = nullptr;
    while (S_OK == enum_pin->Next(1, &pin, NULL)) {
        PIN_DIRECTION pPinDir;
        pin->QueryDirection(&pPinDir);
        if (PINDIR_OUTPUT == pPinDir)  // This is an output pin
        {
            if (category == GUID_NULL || PinMatchesCategory(pin, category)) {
                enum_pin->Release();
                return pin;
            }
        }
        pin->Release();
        pin = nullptr;
    }
    enum_pin->Release();
    return nullptr;
}

IPin* GetInputPin(IBaseFilter* filter) {
    IEnumPins* enum_pin = nullptr;
    filter->EnumPins(&enum_pin);
    if (!enum_pin) {
        return nullptr;
    }

    // get first unconnected pin
    HRESULT hr = enum_pin->Reset();  // set to first pin
    IPin* pin = nullptr;
    while (S_OK == enum_pin->Next(1, &pin, NULL)) {
        PIN_DIRECTION pPinDir;
        pin->QueryDirection(&pPinDir);
        if (PINDIR_INPUT == pPinDir)  // This is an input pin
        {
            IPin* temp_pin = nullptr;
            if (S_OK != pin->ConnectedTo(&temp_pin))  // The pint is not connected
            {
                enum_pin->Release();
                return pin;
            }
        }
        pin->Release();
    }
    enum_pin->Release();
    return nullptr;
}

LONGLONG GetMaxOfFrameArray(LONGLONG* max_fps_array, long size) {
    LONGLONG max_fps = max_fps_array[0];
    for (int i = 0; i < size; i++) {
        if (max_fps > max_fps_array[i]) {
            max_fps = max_fps_array[i];
        }
    }
    return max_fps;
}

void FreeMediaType(AM_MEDIA_TYPE* media_type) {
    if (!media_type)
        return;
    ResetMediaType(media_type);
    CoTaskMemFree(media_type);
}

void ResetMediaType(AM_MEDIA_TYPE* media_type) {
    if (!media_type)
        return;
    if (media_type->cbFormat != 0) {
        CoTaskMemFree(media_type->pbFormat);
        media_type->cbFormat = 0;
        media_type->pbFormat = nullptr;
    }
    if (media_type->pUnk) {
        media_type->pUnk->Release();
        media_type->pUnk = nullptr;
    }
}

VideoType GetVideoType(GUID type) {
    VideoType video_type = VideoType::kVideoTypeUnknown;
    if (type == MEDIASUBTYPE_I420) {
        video_type = VideoType::kVideoTypeI420;
    }
    else if (type == MEDIASUBTYPE_IYUV) {
        video_type = VideoType::kVideoTypeIYUV;
    }
    else if (type == MEDIASUBTYPE_RGB24) {
        video_type = VideoType::kVideoTypeRGB24;
    }
    else if (type == MEDIASUBTYPE_YUY2) {
        video_type = VideoType::kVideoTypeYUY2;
    }
    else if (type == MEDIASUBTYPE_RGB565) {
        video_type = VideoType::kVideoTypeRGB565;
    }
    else if (type == MEDIASUBTYPE_MJPG) {
        video_type = VideoType::kVideoTypeMJPEG;
    }
    else if (type == MEDIASUBTYPE_dvsl || type == MEDIASUBTYPE_dvsd || type == MEDIASUBTYPE_dvhd) {
        // If this is an external DV camera
        video_type = VideoType::kVideoTypeYUY2;  // MS DV filter seems to create this type
    }
    else if (type == MEDIASUBTYPE_UYVY) {
        // Seen used by Declink capture cards
        video_type = VideoType::kVideoTypeUYVY;
    }
    else if (type == MEDIASUBTYPE_HDYC) {
        video_type = VideoType::kVideoTypeUYVY;
    }
    return video_type;
}

BOOL PinMatchesCategory(IPin* pin, REFGUID category) {
    BOOL found = FALSE;
    IKsPropertySet* ks = NULL;
    HRESULT hr = pin->QueryInterface(IID_PPV_ARGS(&ks));
    if (FAILED(hr)) {
        return found;
    }
    GUID pin_category;
    DWORD ret;
    hr = ks->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, NULL, 0, &pin_category, sizeof(GUID), &ret);
    if (SUCCEEDED(hr) && (ret == sizeof(GUID))) {
        found = (pin_category == category);
    }
    ks->Release();
    return found;
}

void GetSampleProperties(IMediaSample* sample, AM_SAMPLE2_PROPERTIES* props) {
    //  Get the properties the hard way.
    props->cbData = sizeof(*props);
    props->dwTypeSpecificFlags = 0;
    props->dwStreamId = AM_STREAM_MEDIA;
    props->dwSampleFlags = 0;

    if (sample->IsDiscontinuity() == S_OK)
        props->dwSampleFlags |= AM_SAMPLE_DATADISCONTINUITY;

    if (sample->IsPreroll() == S_OK)
        props->dwSampleFlags |= AM_SAMPLE_PREROLL;

    if (sample->IsSyncPoint() == S_OK)
        props->dwSampleFlags |= AM_SAMPLE_SPLICEPOINT;

    if (SUCCEEDED(sample->GetTime(&props->tStart, &props->tStop)))
        props->dwSampleFlags |= AM_SAMPLE_TIMEVALID | AM_SAMPLE_STOPVALID;

    if (sample->GetMediaType(&props->pMediaType) == S_OK)
        props->dwSampleFlags |= AM_SAMPLE_TYPECHANGED;

    sample->GetPointer(&props->pbBuffer);
    props->lActual = sample->GetActualDataLength();
    props->cbBuffer = sample->GetSize();
}

HRESULT CopyMediaType(AM_MEDIA_TYPE* target, const AM_MEDIA_TYPE* source) {
    *target = *source;
    if (source->cbFormat != 0) {
        target->pbFormat =
            reinterpret_cast<BYTE*>(CoTaskMemAlloc(source->cbFormat));
        if (target->pbFormat == nullptr) {
            target->cbFormat = 0;
            return E_OUTOFMEMORY;
        }
        else {
            CopyMemory(target->pbFormat, source->pbFormat, target->cbFormat);
        }
    }

    if (target->pUnk != nullptr)
        target->pUnk->AddRef();

    return S_OK;
}

bool TranslateMediaTypeToVideoCaptureCapability(const AM_MEDIA_TYPE* media_type, VideoDescription* capability) {
    if (!media_type || media_type->majortype != MEDIATYPE_Video ||
        !media_type->pbFormat) {
        return false;
    }

    const BITMAPINFOHEADER* bih = nullptr;
    if (media_type->formattype == FORMAT_VideoInfo) {
        bih = &reinterpret_cast<VIDEOINFOHEADER*>(media_type->pbFormat)->bmiHeader;
    }
    else if (media_type->formattype != FORMAT_VideoInfo2) {
        bih = &reinterpret_cast<VIDEOINFOHEADER2*>(media_type->pbFormat)->bmiHeader;
    }
    else {
        return false;
    }

    const GUID& sub_type = media_type->subtype;
    if (sub_type == MEDIASUBTYPE_MJPG &&
        bih->biCompression == MAKEFOURCC('M', 'J', 'P', 'G')) {
        capability->video_type = VideoType::kVideoTypeMJPEG;
    }
    else if (sub_type == MEDIASUBTYPE_I420 &&
        bih->biCompression == MAKEFOURCC('I', '4', '2', '0')) {
        capability->video_type = VideoType::kVideoTypeI420;
    }
    else if (sub_type == MEDIASUBTYPE_YUY2 &&
        bih->biCompression == MAKEFOURCC('Y', 'U', 'Y', '2')) {
        capability->video_type = VideoType::kVideoTypeYUY2;
    }
    else if (sub_type == MEDIASUBTYPE_UYVY &&
        bih->biCompression == MAKEFOURCC('U', 'Y', 'V', 'Y')) {
        capability->video_type = VideoType::kVideoTypeUYVY;
    }
    else if (sub_type == MEDIASUBTYPE_HDYC) {
        capability->video_type = VideoType::kVideoTypeUYVY;
    }
    else if (sub_type == MEDIASUBTYPE_RGB24 && bih->biCompression == BI_RGB) {
        capability->video_type = VideoType::kVideoTypeRGB24;
    }
    else {
        return false;
    }

    // Store the incoming width and height
    capability->width = bih->biWidth;
    // Store the incoming height,
    // for RGB24 we assume the frame to be upside down
    if (sub_type == MEDIASUBTYPE_RGB24 && bih->biHeight > 0) {
        capability->height = -(bih->biHeight);
    }
    else {
        capability->height = abs(bih->biHeight);
    }
    return true;
}

bool IsMediaTypeFullySpecified(const AM_MEDIA_TYPE& type) {
    return type.majortype != GUID_NULL && type.formattype != GUID_NULL;
}

bool IsMediaTypePartialMatch(const AM_MEDIA_TYPE& a, const AM_MEDIA_TYPE& b) {
    if (b.majortype != GUID_NULL && a.majortype != b.majortype)
        return false;

    if (b.subtype != GUID_NULL && a.subtype != b.subtype)
        return false;

    if (b.formattype != GUID_NULL) {
        // if the format block is specified then it must match exactly
        if (a.formattype != b.formattype)
            return false;

        if (a.cbFormat != b.cbFormat)
            return false;

        if (a.cbFormat != 0 && memcmp(a.pbFormat, b.pbFormat, a.cbFormat) != 0)
            return false;
    }

    return true;
}

void SetMediaInfoFromVideoType(VideoType video_type,
    BITMAPINFOHEADER* bitmap_header,
    AM_MEDIA_TYPE* media_type) {
    switch (video_type) {
    case VideoType::kVideoTypeI420:
        bitmap_header->biCompression = MAKEFOURCC('I', '4', '2', '0');
        bitmap_header->biBitCount = 12;  // bit per pixel
        media_type->subtype = MEDIASUBTYPE_I420;
        break;
    case VideoType::kVideoTypeYUY2:
        bitmap_header->biCompression = MAKEFOURCC('Y', 'U', 'Y', '2');
        bitmap_header->biBitCount = 16;  // bit per pixel
        media_type->subtype = MEDIASUBTYPE_YUY2;
        break;
    case VideoType::kVideoTypeRGB24:
        bitmap_header->biCompression = BI_RGB;
        bitmap_header->biBitCount = 24;  // bit per pixel
        media_type->subtype = MEDIASUBTYPE_RGB24;
        break;
    case VideoType::kVideoTypeUYVY:
        bitmap_header->biCompression = MAKEFOURCC('U', 'Y', 'V', 'Y');
        bitmap_header->biBitCount = 16;  // bit per pixel
        media_type->subtype = MEDIASUBTYPE_UYVY;
        break;
    case VideoType::kVideoTypeMJPEG:
        bitmap_header->biCompression = MAKEFOURCC('M', 'J', 'P', 'G');
        bitmap_header->biBitCount = 12;  // bit per pixel
        media_type->subtype = MEDIASUBTYPE_MJPG;
        break;
    default:
        break;
    }
}

BYTE* AllocMediaTypeFormatBuffer(AM_MEDIA_TYPE* media_type, ULONG length) {
    if (media_type->cbFormat == length)
        return media_type->pbFormat;

    BYTE* buffer = static_cast<BYTE*>(CoTaskMemAlloc(length));
    if (!buffer)
        return nullptr;

    if (media_type->pbFormat) {
        CoTaskMemFree(media_type->pbFormat);
        media_type->pbFormat = nullptr;
    }

    media_type->cbFormat = length;
    media_type->pbFormat = buffer;
    return buffer;
}
