#include "sink_filter_ds.h"

#include <dvdmedia.h>  // VIDEOINFOHEADER2
#include <initguid.h>
#include <algorithm>
#include <list>
#include <iostream>

#include "dshow_utils.h"


MediaTypesEnum::MediaTypesEnum(const VideoDescription& video_description) : video_description_(video_description) {
    video_type_list_.push_back(VideoType::kVideoTypeI420);
    video_type_list_.push_back(VideoType::kVideoTypeYUY2);
    video_type_list_.push_back(VideoType::kVideoTypeRGB24);
    video_type_list_.push_back(VideoType::kVideoTypeUYVY);
    video_type_list_.push_back(VideoType::kVideoTypeMJPEG);

    auto it = std::find(video_type_list_.begin(), video_type_list_.end(), video_description_.video_type);
    if (it != video_type_list_.end()) {
        if (it != video_type_list_.begin()) {
            video_type_list_.splice(video_type_list_.begin(), video_type_list_, it, std::next(it));
        }
    }
}

MediaTypesEnum::~MediaTypesEnum() = default;

STDMETHODIMP MediaTypesEnum::QueryInterface(REFIID riid, void** ppv) {
    if (riid == IID_IUnknown || riid == IID_IEnumMediaTypes) {
        *ppv = static_cast<IEnumMediaTypes*>(this);
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP MediaTypesEnum::Clone(IEnumMediaTypes** pins) {
    return E_NOTIMPL;
}

STDMETHODIMP MediaTypesEnum::Next(ULONG count, AM_MEDIA_TYPE** types, ULONG* fetched) {
    if (fetched)
        *fetched = 0;

    for (ULONG i = 0;
        i < count && pos_ < static_cast<int>(video_type_list_.size());
        ++i) {
        AM_MEDIA_TYPE* media_type = reinterpret_cast<AM_MEDIA_TYPE*>(
            CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE)));
        ZeroMemory(media_type, sizeof(*media_type));
        types[i] = media_type;
        VIDEOINFOHEADER* vih = reinterpret_cast<VIDEOINFOHEADER*>(
            AllocMediaTypeFormatBuffer(media_type, sizeof(VIDEOINFOHEADER)));
        ZeroMemory(vih, sizeof(*vih));
        vih->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        vih->bmiHeader.biPlanes = 1;
        vih->bmiHeader.biClrImportant = 0;
        vih->bmiHeader.biClrUsed = 0;
        if (video_description_.fps != 0)
            vih->AvgTimePerFrame = 10000000 / video_description_.fps;

        SetRectEmpty(&vih->rcSource);  // we want the whole image area rendered.
        SetRectEmpty(&vih->rcTarget);  // no particular destination rectangle

        media_type->majortype = MEDIATYPE_Video;
        media_type->formattype = FORMAT_VideoInfo;
        media_type->bTemporalCompression = FALSE;

        // Set format information.
        auto format_it = std::next(video_type_list_.begin(), pos_++);
        SetMediaInfoFromVideoType(*format_it, &vih->bmiHeader, media_type);

        vih->bmiHeader.biWidth = video_description_.width;
        vih->bmiHeader.biHeight = video_description_.height;
        vih->bmiHeader.biSizeImage = ((vih->bmiHeader.biBitCount / 4) *
            video_description_.height * video_description_.width) /
            2;

        media_type->lSampleSize = vih->bmiHeader.biSizeImage;
        media_type->bFixedSizeSamples = true;
        if (fetched) {
            ++(*fetched);
        }
    }
    return pos_ == static_cast<int>(video_type_list_.size()) ? S_FALSE : S_OK;
}

STDMETHODIMP MediaTypesEnum::Reset() {
    pos_ = 0;
    return S_OK;
}

STDMETHODIMP MediaTypesEnum::Skip(ULONG cMediaTypes) {
    return E_NOTIMPL;
}

STDMETHODIMP_(ULONG) MediaTypesEnum::AddRef() {
    return InterlockedIncrement(&m_cRef);

}

STDMETHODIMP_(ULONG) MediaTypesEnum::Release() {
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0) {
        delete this;
    }
    return cRef;
}

CEnumPins::CEnumPins(IPin* pin) : pin_(pin) {

}

CEnumPins::~CEnumPins() {

}

STDMETHODIMP CEnumPins::QueryInterface(REFIID riid, void** ppv) {
    if (riid == IID_IUnknown || riid == IID_IEnumPins) {
        *ppv = static_cast<IEnumPins*>(this);
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP CEnumPins::Clone(IEnumPins** pins) {
    return E_NOTIMPL;
}

STDMETHODIMP CEnumPins::Next(ULONG count, IPin** pins, ULONG* fetched) {
    if (pos_ > 0) {
        if (fetched)
            *fetched = 0;
        return S_FALSE;
    }

    ++pos_;
    pins[0] = pin_;
    pins[0]->AddRef();
    if (fetched) {
        *fetched = 1;
    }
    return count == 1 ? S_OK : S_FALSE;
}

STDMETHODIMP CEnumPins::Skip(ULONG count) {
    return E_NOTIMPL;
}

STDMETHODIMP CEnumPins::Reset() {
    pos_ = 0;
    return S_OK;
}

STDMETHODIMP_(ULONG) CEnumPins::AddRef() {
    return InterlockedIncrement(&m_cRef);

}
STDMETHODIMP_(ULONG) CEnumPins::Release() {
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0) {
        delete this;
    }
    return cRef;
}


CaptureInputPin::CaptureInputPin(CaptureSinkFilter* filter) {
    info_.pFilter = filter;
    info_.dir = PINDIR_INPUT;
}

CaptureInputPin::~CaptureInputPin() {
    ResetMediaType(&media_type_);
}

HRESULT CaptureInputPin::SetRequestedCapability(const VideoDescription& video_description) {
    request_format_ = video_description;
    return S_OK;
}

void CaptureInputPin::OnFilterActivated() {
    runtime_error_ = false;
    flushing_ = false;
    /*capture_checker_.Detach();
    capture_thread_id_ = 0;*/
}

void CaptureInputPin::OnFilterDeactivated() {
    flushing_ = true;
    if (allocator_) {
        allocator_->Decommit();
    }
}

CaptureSinkFilter* CaptureInputPin::Filter() const {
    return static_cast<CaptureSinkFilter*>(info_.pFilter);
}

HRESULT CaptureInputPin::AttemptConnection(IPin* receive_pin, const AM_MEDIA_TYPE* media_type) {
    HRESULT hr = CheckDirection(receive_pin);
    if (FAILED(hr))
        return hr;

    if (!TranslateMediaTypeToVideoCaptureCapability(media_type,
        &resulting_format_)) {
        ClearAllocator(true);
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    // See if the other pin will accept this type.
    hr = receive_pin->ReceiveConnection(static_cast<IPin*>(this), media_type);
    if (FAILED(hr)) {
        receive_pin_ = nullptr;  // Should already be null, but just in case.
        return hr;
    }
    ResetMediaType(&media_type_);
    CopyMediaType(&media_type_, media_type);

    return S_OK;
}

std::vector<AM_MEDIA_TYPE*> CaptureInputPin::DetermineCandidateFormats(
    IPin* receive_pin, const AM_MEDIA_TYPE* media_type) {
    std::vector<AM_MEDIA_TYPE*> ret;

    for (int i = 0; i < 2; i++) {
        IEnumMediaTypes* types = nullptr;
        if (i == 0) {
            // First time around, try types from receive_pin.
            receive_pin->EnumMediaTypes(&types);
        }
        else {
            // Then try ours.
            EnumMediaTypes(&types);
        }

        if (types) {
            while (true) {
                ULONG fetched = 0;
                AM_MEDIA_TYPE* this_type = nullptr;
                if (types->Next(1, &this_type, &fetched) != S_OK)
                    break;

                if (IsMediaTypePartialMatch(*this_type, *media_type)) {
                    ret.push_back(this_type);
                }
                else {
                    FreeMediaType(this_type);
                }
            }
            types->Release();
        }
    }

    return ret;
}

void CaptureInputPin::ClearAllocator(bool decommit) {
    if (!allocator_) {
        return;
    }
    if (decommit) {
        allocator_->Decommit();
    }
    allocator_ = nullptr;
}

HRESULT CaptureInputPin::CheckDirection(IPin* pin) const {
    PIN_DIRECTION pd;
    pin->QueryDirection(&pd);
    // Fairly basic check, make sure we don't pair input with input etc.
    return pd == info_.dir ? VFW_E_INVALID_DIRECTION : S_OK;
}

STDMETHODIMP CaptureInputPin::QueryInterface(REFIID riid, void** ppv) {
    (*ppv) = nullptr;
    if (riid == IID_IUnknown || riid == IID_IMemInputPin) {
        *ppv = static_cast<IMemInputPin*>(this);
    }
    else if (riid == IID_IPin) {
        *ppv = static_cast<IPin*>(this);
    }

    if (!(*ppv))
        return E_NOINTERFACE;

    static_cast<IMemInputPin*>(this)->AddRef();
    return S_OK;
}

STDMETHODIMP CaptureInputPin::Connect(IPin* receive_pin, const AM_MEDIA_TYPE* media_type) {
    if (!media_type || !receive_pin)
        return E_POINTER;

    if (!Filter()->IsStopped())
        return VFW_E_NOT_STOPPED;

    if (receive_pin_) {
        return VFW_E_ALREADY_CONNECTED;
    }

    if (IsMediaTypeFullySpecified(*media_type))
        return AttemptConnection(receive_pin, media_type);

    auto types = DetermineCandidateFormats(receive_pin, media_type);
    bool connected = false;
    for (auto* type : types) {
        if (!connected && AttemptConnection(receive_pin, media_type) == S_OK)
            connected = true;

        FreeMediaType(type);
    }

    return connected ? S_OK : VFW_E_NO_ACCEPTABLE_TYPES;
}

STDMETHODIMP CaptureInputPin::ReceiveConnection(IPin* connector, const AM_MEDIA_TYPE* media_type) {
    if (receive_pin_) {
        return VFW_E_ALREADY_CONNECTED;
    }

    HRESULT hr = CheckDirection(connector);
    if (FAILED(hr))
        return hr;

    if (!TranslateMediaTypeToVideoCaptureCapability(media_type, &resulting_format_))
        return VFW_E_TYPE_NOT_ACCEPTED;

    // Complete the connection

    receive_pin_ = connector;
    ResetMediaType(&media_type_);
    CopyMediaType(&media_type_, media_type);

    return S_OK;
}

STDMETHODIMP CaptureInputPin::Disconnect() {
    if (!Filter()->IsStopped())
        return VFW_E_NOT_STOPPED;

    if (!receive_pin_)
        return S_FALSE;

    ClearAllocator(true);
    receive_pin_ = nullptr;

    return S_OK;
}

STDMETHODIMP CaptureInputPin::ConnectedTo(IPin** pin) {
    if (!receive_pin_)
        return VFW_E_NOT_CONNECTED;

    *pin = receive_pin_;
    receive_pin_->AddRef();

    return S_OK;
}

STDMETHODIMP CaptureInputPin::ConnectionMediaType(AM_MEDIA_TYPE* media_type) {
    if (!receive_pin_) {
        return VFW_E_NOT_CONNECTED;
    }
    CopyMediaType(media_type, &media_type_);

    return S_OK;
}

STDMETHODIMP CaptureInputPin::QueryPinInfo(PIN_INFO* info) {
    *info = info_;
    if (info_.pFilter) {
        info_.pFilter->AddRef();
    }
    return S_OK;
}

STDMETHODIMP CaptureInputPin::QueryDirection(PIN_DIRECTION* pin_dir) {
    *pin_dir = info_.dir;
    return S_OK;
}

STDMETHODIMP CaptureInputPin::QueryId(LPWSTR* id) {
    size_t len = lstrlenW(info_.achName);
    *id = reinterpret_cast<LPWSTR>(CoTaskMemAlloc((len + 1) * sizeof(wchar_t)));
    lstrcpyW(*id, info_.achName);
    return S_OK;
}

STDMETHODIMP CaptureInputPin::QueryAccept(const AM_MEDIA_TYPE* media_type) {
    VideoDescription capability(resulting_format_);
    return TranslateMediaTypeToVideoCaptureCapability(media_type, &capability)
        ? S_FALSE : S_OK;
}

STDMETHODIMP CaptureInputPin::EnumMediaTypes(IEnumMediaTypes** types) {
    *types = new MediaTypesEnum(request_format_);
    (*types)->AddRef();
    return S_OK;
}

STDMETHODIMP CaptureInputPin::QueryInternalConnections(IPin** pins, ULONG* count) {
    return E_NOTIMPL;
}

STDMETHODIMP CaptureInputPin::EndOfStream() {
    return S_OK;
}

STDMETHODIMP CaptureInputPin::BeginFlush() {
    flushing_ = true;
    return S_OK;
}

STDMETHODIMP CaptureInputPin::EndFlush() {
    flushing_ = false;
    runtime_error_ = false;
    return S_OK;
}

STDMETHODIMP CaptureInputPin::NewSegment(REFERENCE_TIME start, REFERENCE_TIME stop, double rate) {
    return S_OK;
}

STDMETHODIMP CaptureInputPin::GetAllocator(IMemAllocator** allocator) {
    if (allocator_ == nullptr) {
        HRESULT hr = CoCreateInstance(CLSID_MemoryAllocator, 0,
            CLSCTX_INPROC_SERVER, IID_IMemAllocator,
            reinterpret_cast<void**>(allocator_));
        if (FAILED(hr))
            return hr;
    }
    *allocator = allocator_;
    allocator_->AddRef();
    return S_OK;
}

STDMETHODIMP CaptureInputPin::NotifyAllocator(IMemAllocator* allocator, BOOL read_only) {
    allocator_ = allocator;
    if (allocator_) {
        allocator_->AddRef();
    }
    if (allocator) {
        allocator->Release();
    }
    /*IMemAllocator *pOldAllocator = allocator_;
    allocator_->AddRef();

    allocator_ = allocator;

    if (pOldAllocator != NULL) {
        pOldAllocator->Release();
    }*/

    return S_OK;
}

STDMETHODIMP CaptureInputPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES* props) {
    return E_NOTIMPL;
}

STDMETHODIMP CaptureInputPin::Receive(IMediaSample* sample) {
    CaptureSinkFilter* const filter = static_cast<CaptureSinkFilter*>(Filter());

    if (flushing_.load(std::memory_order_relaxed))
        return S_FALSE;

    if (runtime_error_.load(std::memory_order_relaxed))
        return VFW_E_RUNTIME_ERROR;

    if (!capture_thread_id_) {
        // Make sure we set the thread name only once.
        capture_thread_id_ = GetCurrentThreadId();
    }

    AM_SAMPLE2_PROPERTIES sample_props = {};
    GetSampleProperties(sample, &sample_props);
    // Has the format changed in this sample?
    if (sample_props.dwSampleFlags & AM_SAMPLE_TYPECHANGED) {
        if (!TranslateMediaTypeToVideoCaptureCapability(sample_props.pMediaType, &resulting_format_)) {
            // Raise a runtime error if we fail the media type
            runtime_error_ = true;
            EndOfStream();
            Filter()->NotifyEvent(EC_ERRORABORT, VFW_E_TYPE_NOT_ACCEPTED, 0);
            return VFW_E_INVALIDMEDIATYPE;
        }
    }

    filter->ProcessCapturedFrame(sample_props.pbBuffer, sample_props.lActual, resulting_format_);

    return S_OK;
}

STDMETHODIMP CaptureInputPin::ReceiveMultiple(IMediaSample** samples, long count, long* processed) {
    HRESULT hr = S_OK;
    *processed = 0;
    while (count-- > 0) {
        hr = Receive(samples[*processed]);
        if (hr != S_OK)
            break;
        ++(*processed);
    }
    return hr;
}

STDMETHODIMP CaptureInputPin::ReceiveCanBlock() {
    return S_FALSE;
}

STDMETHODIMP_(ULONG) CaptureInputPin::AddRef() {
    return InterlockedIncrement(&m_cRef);

}
STDMETHODIMP_(ULONG) CaptureInputPin::Release() {
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0) {
        delete this;
    }
    return cRef;
}

CaptureSinkFilter::CaptureSinkFilter() :
    input_pin_(new CaptureInputPin(this)) {
}

CaptureSinkFilter::~CaptureSinkFilter() {

}

HRESULT CaptureSinkFilter::SetVideoDescription(const VideoDescription& video_description) {
    return input_pin_->SetRequestedCapability(video_description);
}

void CaptureSinkFilter::ProcessCapturedFrame(unsigned char* buffer, size_t length, VideoDescription video_description) {
    if (callback_) {
        callback_(buffer, length, video_description);
    }
}

void CaptureSinkFilter::NotifyEvent(long code, LONG_PTR param1, LONG_PTR param2) {
    if (!sink_)
        return;

    if (EC_COMPLETE == code) {
        param2 = reinterpret_cast<LONG_PTR>(static_cast<IBaseFilter*>(this));
    }
    sink_->Notify(code, param1, param2);
}

bool CaptureSinkFilter::IsStopped() const {
    return state_ == State_Stopped;
}

void CaptureSinkFilter::RegisterCallback(FrameCallback callback) {
    callback_ = callback;
}

STDMETHODIMP CaptureSinkFilter::QueryInterface(REFIID riid, void** ppv) {
    if (riid == IID_IUnknown || riid == IID_IPersist || riid == IID_IBaseFilter) {
        *ppv = static_cast<IBaseFilter*>(this);
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

// IPersist
STDMETHODIMP CaptureSinkFilter::GetClassID(CLSID* clsid) {
    *clsid = CLSID_SINKFILTER;
    return S_OK;
}

// IMediaFilter.
STDMETHODIMP CaptureSinkFilter::GetState(DWORD msecs, FILTER_STATE* state) {
    *state = state_;
    return S_OK;
}

STDMETHODIMP CaptureSinkFilter::SetSyncSource(IReferenceClock* clock) {
    return S_OK;
}

STDMETHODIMP CaptureSinkFilter::GetSyncSource(IReferenceClock** clock) {
    return E_NOTIMPL;
}

STDMETHODIMP CaptureSinkFilter::Pause() {
    state_ = State_Paused;
    return S_OK;
}

STDMETHODIMP CaptureSinkFilter::Run(REFERENCE_TIME start) {
    if (state_ == State_Stopped) {
        Pause();
    }

    state_ = State_Running;
    input_pin_->OnFilterActivated();

    return S_OK;
}

STDMETHODIMP CaptureSinkFilter::Stop() {
    if (state_ == State_Stopped) {
        return S_OK;
    }

    state_ = State_Stopped;
    input_pin_->OnFilterDeactivated();

    return S_OK;
}

// IBaseFilter
STDMETHODIMP CaptureSinkFilter::EnumPins(IEnumPins** pins) {

    *pins = new CEnumPins(input_pin_);
    (*pins)->AddRef();
    return S_OK;
}

STDMETHODIMP CaptureSinkFilter::FindPin(LPCWSTR id, IPin** pin) {
    return VFW_E_NOT_FOUND;
}

STDMETHODIMP CaptureSinkFilter::QueryFilterInfo(FILTER_INFO* info) {
    *info = info_;
    if (info->pGraph) {
        info->pGraph->AddRef();
    }
    return S_OK;
}

STDMETHODIMP CaptureSinkFilter::JoinFilterGraph(IFilterGraph* graph, LPCWSTR name) {

    info_.pGraph = graph;  // No AddRef().
    sink_ = nullptr;
    if (info_.pGraph) {
        HRESULT hr = info_.pGraph->QueryInterface(IID_IMediaEventSink, (void**)&sink_);
        if (FAILED(hr)) {
            std::cout << "Failed get MediaEventSink" << std::endl;
        }
        else {
            sink_->Release();
        }
    }
    info_.achName[0] = L'\0';
    if (name) {
        size_t namelen;
        HRESULT hr = StringCchLengthW(name, STRSAFE_MAX_CCH, &namelen);

        lstrcpynW(info_.achName, name, namelen);
    }
    return S_OK;
}

STDMETHODIMP CaptureSinkFilter::QueryVendorInfo(LPWSTR* vendor_info) {
    return E_NOTIMPL;
}

STDMETHODIMP_(ULONG) CaptureSinkFilter::AddRef() {
    return InterlockedIncrement(&m_cRef);

}
STDMETHODIMP_(ULONG) CaptureSinkFilter::Release() {
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0) {
        delete this;
    }
    return cRef;
}