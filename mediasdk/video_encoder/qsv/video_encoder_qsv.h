#pragma once
#include <vector>
#include <memory>
#include "video_encoder.h"
#include "mfx/mfxvideo++.h"
#include "d3d11_device.h"
#include "d3d11_allocator.h"
#include "d3d_allocator.h"
#include "d3d_device.h"
#include "general_allocator.h"
#include "sysmem_allocator.h"

enum MemType {
    kMemTypeD3D11,
    kMemTypeD3D9,
    kMemTypeSystem,
};

class VideoEncoderQSV : public VideoEncoder {
public:
    VideoEncoderQSV();
    ~VideoEncoderQSV();

    void EncodeFrame(std::shared_ptr<VideoFrame> video_frame, bool keyframe) override;

    static bool IsCompatible();

private:
    bool Init();
    bool Uninit();

    mfxStatus CreateAllocator();
    mfxStatus InitEnocdeParams();
    mfxStatus AllocFrames();
    mfxStatus DeleteFrames();
    mfxStatus CreateHWDevice();
    mfxStatus InitBitstream();

private:
    bool init_{};
    std::vector<uint8_t> buffer_{};
    uint32_t buffer_size_{};

    MFXVideoSession mfx_session_{};
    std::unique_ptr<MFXVideoENCODE> mfx_encoder_{};
    std::unique_ptr<CHWDevice> hw_device_{};
    std::unique_ptr<MFXFrameAllocator> mfx_frame_allocator_{};
    std::unique_ptr<mfxAllocatorParams> allocator_params_{};
    std::unique_ptr<mfxFrameSurface1[]> frame_surfaces_{};
    mfxVideoParam encode_param_{};
    uint32_t surface_nums_{};
    mfxExtCodingOption coding_opt_{};
    mfxBitstream output_bitstream_{};
    mfxSyncPoint sync_point_{};
    mfxEncodeCtrl encode_ctrl_{};
    mfxFrameAllocResponse encode_response_{};

    MemType mem_type_ = kMemTypeD3D11;
    bool external_alloc_{};
    bool alloc_frame_{};
};