#pragma once

#include <dshow.h>

#include <atomic>
#include <memory>
#include <vector>
#include <fstream>
#include <list>

#include <functional>
#include "video/video_info.h"

class EnumPins;
class CaptureSinkFilter;

class MediaTypesEnum : public IEnumMediaTypes {
public:
    MediaTypesEnum(const VideoDescription& video_device_info);
    ~MediaTypesEnum();

private:
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override;
    STDMETHOD(Clone)(IEnumMediaTypes** pins) override;
    STDMETHOD(Next)(ULONG count, AM_MEDIA_TYPE** types, ULONG* fetched) override;
    STDMETHOD(Skip)(ULONG count) override;
    STDMETHOD(Reset)() override;
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    LONG m_cRef = 0;


private:
    std::list<VideoType> video_type_list_{};
    VideoDescription video_description_{};
    int pos_{ 0 };
};

class CEnumPins : public IEnumPins {
public:
    CEnumPins(IPin* pin);
    virtual ~CEnumPins();

private:
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
    STDMETHOD(Clone)(IEnumPins** pins);
    STDMETHOD(Next)(ULONG count, IPin** pins, ULONG* fetched);
    STDMETHOD(Skip)(ULONG count);
    STDMETHOD(Reset)();
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

private:
    IPin* pin_{};
    int pos_ = 0;
    LONG m_cRef = 0;
};

class CaptureInputPin : public IMemInputPin, public IPin {
public:
    CaptureInputPin(CaptureSinkFilter* filter);
    ~CaptureInputPin();

    HRESULT SetRequestedCapability(const VideoDescription& video_description);
    void OnFilterActivated();
    void OnFilterDeactivated();

private:
    CaptureSinkFilter* Filter() const;

    HRESULT AttemptConnection(IPin* receive_pin, const AM_MEDIA_TYPE* media_type);
    std::vector<AM_MEDIA_TYPE*> DetermineCandidateFormats(
        IPin* receive_pin,
        const AM_MEDIA_TYPE* media_type);
    void ClearAllocator(bool decommit);
    HRESULT CheckDirection(IPin* pin) const;

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override;
    STDMETHOD(Connect)(IPin* receive_pin, const AM_MEDIA_TYPE* media_type) override;
    STDMETHOD(ReceiveConnection)(IPin* connector, const AM_MEDIA_TYPE* media_type) override;
    STDMETHOD(Disconnect)() override;
    STDMETHOD(ConnectedTo)(IPin** pin) override;
    STDMETHOD(ConnectionMediaType)(AM_MEDIA_TYPE* media_type) override;
    STDMETHOD(QueryPinInfo)(PIN_INFO* info) override;
    STDMETHOD(QueryDirection)(PIN_DIRECTION* pin_dir) override;
    STDMETHOD(QueryId)(LPWSTR* id) override;
    STDMETHOD(QueryAccept)(const AM_MEDIA_TYPE* media_type) override;
    STDMETHOD(EnumMediaTypes)(IEnumMediaTypes** types) override;
    STDMETHOD(QueryInternalConnections)(IPin** pins, ULONG* count) override;
    STDMETHOD(EndOfStream)() override;
    STDMETHOD(BeginFlush)() override;
    STDMETHOD(EndFlush)() override;
    STDMETHOD(NewSegment)(REFERENCE_TIME start, REFERENCE_TIME stop, double rate) override;
    // IMemInputPin
    STDMETHOD(GetAllocator)(IMemAllocator** allocator) override;
    STDMETHOD(NotifyAllocator)(IMemAllocator* allocator, BOOL read_only) override;
    STDMETHOD(GetAllocatorRequirements)(ALLOCATOR_PROPERTIES* props) override;
    STDMETHOD(Receive)(IMediaSample* sample) override;
    STDMETHOD(ReceiveMultiple)(IMediaSample** samples, long count, long* processed) override;
    STDMETHOD(ReceiveCanBlock)() override;

    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

private:
    VideoDescription request_format_{};
    VideoDescription resulting_format_{};
    DWORD capture_thread_id_{ 0 };
    IMemAllocator* allocator_{};
    IPin* receive_pin_{};
    VideoDescription video_description;

    std::atomic_bool flushing_{ false };
    std::atomic_bool runtime_error_{ false };
    PIN_INFO info_ = {};
    AM_MEDIA_TYPE media_type_{};
    LONG m_cRef = 0;
};

class CaptureSinkFilter : public IBaseFilter {
public:
    using FrameCallback = std::function<void(unsigned char* pBuffer, size_t length, const VideoDescription& video_description)>;
public:
    // 需要一个回调将数据回调出去
    CaptureSinkFilter();
    ~CaptureSinkFilter();
    HRESULT SetVideoDescription(const VideoDescription& capability);
    void ProcessCapturedFrame(unsigned char* buffer, size_t length, VideoDescription video_description);
    void NotifyEvent(long code, LONG_PTR param1, LONG_PTR param2);
    bool IsStopped() const;

    void RegisterCallback(FrameCallback callback);

    STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override;
    STDMETHOD(GetClassID)(CLSID* clsid) override;
    STDMETHOD(GetState)(DWORD msecs, FILTER_STATE* state) override;
    STDMETHOD(SetSyncSource)(IReferenceClock* clock) override;
    STDMETHOD(GetSyncSource)(IReferenceClock** clock) override;
    STDMETHOD(Pause)() override;
    STDMETHOD(Run)(REFERENCE_TIME start) override;
    STDMETHOD(Stop)() override;
    STDMETHOD(EnumPins)(IEnumPins** pins) override;
    STDMETHOD(FindPin)(LPCWSTR id, IPin** pin) override;
    STDMETHOD(QueryFilterInfo)(FILTER_INFO* info) override;
    STDMETHOD(JoinFilterGraph)(IFilterGraph* graph, LPCWSTR name) override;
    STDMETHOD(QueryVendorInfo)(LPWSTR* vendor_info) override;
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

private:
    FILTER_INFO info_{};
    CaptureInputPin* input_pin_{};
    IMediaEventSink* sink_{};
    FILTER_STATE state_;
    LONG m_cRef = 0;

    FrameCallback callback_{};
};