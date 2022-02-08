#pragma once
#include <Windows.h>
#include <cstdint>
#include <memory>
#include <vector>
#include <mfxvideo++.h>
#include <mutex>
#include <thread>

#include <d3d11_device.h>
#include <d3d11_allocator.h>
#include <general_allocator.h>
#include <parameters_dumper.h>
#include <plugin_loader.h>

#include "d3d11_device.h"
#include "d3d11_allocator.h"
#include "d3d_allocator.h"
#include "d3d_device.h"
#include "general_allocator.h"
#include "sysmem_allocator.h"
#include "common/readerwriterqueue.h"

#include "video_decoder.h"

class VideoDecoderQSV : public VideoDecoder {
public:
    VideoDecoderQSV();
    ~VideoDecoderQSV();
    void Decode(uint8_t* data, uint32_t len) override;
    void SetRenderHwnd(HWND hwnd) override;

private:
    void Start();
    void Stop();
    bool Init();
    bool Uninit();
    bool ResetDecoder(std::vector<uint8_t>& stream);
    bool InitDecoderParams();
    bool CreateAllocator();
    bool CreateHWDevice();
    bool AllocFrames();
    bool ReleaseFrames();
    void GetVideoSignalInfo();

private:
    HWND render_window_{};
    mfxIMPL impl_{};
    mfxVersion version_{};
    MFXVideoSession mfx_session_{};
    std::shared_ptr<MFXVideoDECODE> mfx_decoder_{};
    mfxVideoParam video_param_{};
    std::unique_ptr<CHWDevice> hw_device_{};
    std::unique_ptr<GeneralAllocator> frame_allocator_{};
    std::unique_ptr<mfxAllocatorParams> allocator_params_{};
    std::unique_ptr<MFXFrameAllocator> mfx_frame_allocator_{};
    std::unique_ptr<mfxFrameSurface1[]> decode_surface_{};
    mfxFrameAllocResponse alloc_response_{};
    mfxFrameSurface1* out_surface_{};
    mfxU16 surface_nums_{};
    bool init_decoder_{};
    bool need_fetch_info_{};
    std::mutex mtx_;
    mfxExtVideoSignalInfo video_signal_info_{};
    moodycamel::BlockingReaderWriterQueue<std::vector<uint8_t>> stream_buffers_{};
    bool running_{};
    std::thread decode_thread_{};
};