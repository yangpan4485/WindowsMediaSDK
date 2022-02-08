#include "video_encoder_qsv.h"

#include "yuv/libyuv.h"

VideoEncoderQSV::VideoEncoderQSV() {}

VideoEncoderQSV::~VideoEncoderQSV() {}

bool VideoEncoderQSV::IsCompatible() {
    mfxVersion version = {{0, 1}};
    MFXVideoSession session;
    mfxStatus status = session.Init(MFX_IMPL_HARDWARE_ANY, &version);
    return status == MFX_ERR_NONE;
}

void VideoEncoderQSV::EncodeFrame(std::shared_ptr<VideoFrame> video_frame, bool keyframe) {
    int width = video_frame->GetWidth();
    int height = video_frame->GetHeight();
    if (!init_) {
        Init();
    }
    mfxStatus status = MFX_ERR_NONE;
    mfxU16 free_surface_index = GetFreeSurfaceIndex(frame_surfaces_.get(), surface_nums_);
    mfxFrameSurface1 surface = frame_surfaces_[free_surface_index];
    if (mem_type_ != kMemTypeSystem) {
        status = mfx_frame_allocator_->Lock(mfx_frame_allocator_->pthis, surface.Data.MemId,
                                            &(surface.Data));
    }
    bool need_scale = (width != output_width_) || (height != output_height_);
    uint8_t* src = video_frame->GetData();
    if (video_frame->GetFrameType() == kFrameTypeARGB) {
        if (need_scale) {
            uint32_t dst_buffer_size = output_width_ * output_height_ * 4;
            if (dst_buffer_size > buffer_.size()) {
                buffer_.resize(dst_buffer_size);
            }
            int ret = libyuv::ARGBScale(video_frame->GetData(), video_frame->GetPitch(),
                                        video_frame->GetWidth(), video_frame->GetHeight(),
                                        &buffer_[0], output_width_ * 4, output_width_,
                                        output_height_, libyuv::FilterMode::kFilterBox);
            libyuv::ARGBToNV12(&buffer_[0], output_width_ * 4, surface.Data.Y, surface.Data.Pitch,
                               surface.Data.UV, surface.Data.Pitch, output_width_,
                output_height_);
        } else {
            libyuv::ARGBToNV12(video_frame->GetData(), video_frame->GetPitch(), surface.Data.Y,
                               surface.Data.Pitch, surface.Data.UV, surface.Data.Pitch,
                               video_frame->GetWidth(), video_frame->GetHeight());
        }

        /*if (!capture_fout_.is_open()) {
            std::string filename =
                "../../../" + std::to_string(output_width_) + std::to_string(output_height_) + ".yuv";
            capture_fout_.open(filename, std::ios::binary | std::ios::out);
        }
        capture_fout_.write((char*)surface.Data.Y, surface.Data.Pitch * height);
        capture_fout_.write((char*)surface.Data.UV, surface.Data.Pitch * height);*/
    } else {
        if (need_scale) {
            if (buffer_size_ != width * height * 3 / 2) {
                buffer_size_ = width * height * 3 / 2;
                buffer_.resize(buffer_size_);
            }
            uint8_t* dst = &buffer_[0];
            libyuv::I420Scale(
                src, width, src + width * height, width >> 1, src + width * height * 5 / 4,
                width >> 1, width, height, dst, output_width_, dst + output_width_ * output_height_,
                output_width_ >> 1, dst + output_width_ * output_height_ * 5 / 4,
                output_width_ >> 1, output_width_, output_height_, libyuv::FilterMode::kFilterBox);

            libyuv::I420ToNV12(dst, output_width_, dst + output_width_ * output_height_,
                               output_width_ >> 1, dst + output_width_ * output_height_ * 5 / 4,
                               output_width_ >> 1, surface.Data.Y, surface.Data.Pitch,
                               surface.Data.UV, surface.Data.Pitch, output_width_, output_height_);
        } else {
            libyuv::I420ToNV12(src, width, src + height * width, width >> 1,
                               src + height * width * 5 / 4, width >> 1, surface.Data.Y,
                               surface.Data.Pitch, surface.Data.UV, surface.Data.Pitch,
                               output_width_, output_height_);
        }
    }
    if (mem_type_ != kMemTypeSystem) {
        status = mfx_frame_allocator_->Unlock(mfx_frame_allocator_->pthis, surface.Data.MemId,
                                              &(surface.Data));
    }
    if (keyframe) {
        encode_ctrl_.FrameType = MFX_FRAMETYPE_I | MFX_FRAMETYPE_IDR | MFX_FRAMETYPE_REF;
    } else {
        encode_ctrl_.FrameType = MFX_FRAMETYPE_UNKNOWN;
    }
    output_bitstream_.DataOffset = 0;
    output_bitstream_.DataLength = 0;
    output_bitstream_.Data = new uint8_t[output_bitstream_.MaxLength];
    for (;;) {
        // Encode a frame asynchronously (returns immediately)
        status = mfx_encoder_->EncodeFrameAsync(&encode_ctrl_, &surface, &output_bitstream_,
                                                &sync_point_);
        if (MFX_ERR_NONE < status && !sync_point_) {
            // Repeat the call if warning and no output
            if (MFX_WRN_DEVICE_BUSY == status)
                MSDK_SLEEP(1); // Wait if device is busy, then repeat the same
                               // call
        } else if (MFX_ERR_NONE < status && sync_point_) {
            status = MFX_ERR_NONE; // Ignore warnings if output is available
            break;
        } else if (MFX_ERR_NOT_ENOUGH_BUFFER == status) {
            // Allocate more bitstream buffer memory here if needed...
            break;
        } else
            break;
    }
    do {
        status = mfx_session_.SyncOperation(sync_point_, 60000);
    } while (status == MFX_WRN_IN_EXECUTION);
    if (callback_) {
        callback_(output_bitstream_.Data, output_bitstream_.DataLength);
        delete[] output_bitstream_.Data;
    }
}

bool VideoEncoderQSV::Init() {
    mfxVersion version = {{0, 1}};
    mfxStatus status = MFX_ERR_NONE;
    // MFX_IMPL_VIA_D3D9, MFX_IMPL_VIA_D3D11
    mfxIMPL impl = MFX_IMPL_HARDWARE_ANY | MFX_IMPL_VIA_D3D11;
    status = mfx_session_.Init(impl, &version);
    mfx_encoder_.reset(new MFXVideoENCODE(mfx_session_));
    status = CreateAllocator();
    // 初始化编码的参数
    status = InitEnocdeParams();
    // 查询需要多少个 surface
    status = mfx_encoder_->Query(&encode_param_, &encode_param_);
    status = AllocFrames();
    status = mfx_encoder_->Init(&encode_param_);
    status = InitBitstream();
    init_ = true;
    return true;
}

bool VideoEncoderQSV::Uninit() {
    init_ = false;
    return init_;
}

mfxStatus VideoEncoderQSV::CreateAllocator() {
    mfxStatus status = MFX_ERR_NONE;
    if (kMemTypeD3D11 == mem_type_ || kMemTypeD3D9 == mem_type_) {
        status = CreateHWDevice();
        mfxHDL hdl = NULL;
        mfxHandleType hdl_t =
            kMemTypeD3D11 == mem_type_ ? MFX_HANDLE_D3D11_DEVICE : MFX_HANDLE_D3D9_DEVICE_MANAGER;
        status = hw_device_->GetHandle(hdl_t, &hdl);
        mfxIMPL impl = 0;
        mfx_session_.QueryIMPL(&impl);
        if (impl != MFX_IMPL_SOFTWARE) {
            status = mfx_session_.SetHandle(hdl_t, hdl);
            MSDK_CHECK_STATUS(status, "m_mfxSession.SetHandle failed");
        }
        if (kMemTypeD3D11 == mem_type_) {
            mfx_frame_allocator_.reset(new D3D11FrameAllocator());
            D3D11AllocatorParams* pd3dAllocParams = new D3D11AllocatorParams;
            pd3dAllocParams->pDevice = reinterpret_cast<ID3D11Device*>(hdl);
            allocator_params_.reset(pd3dAllocParams);
        } else {
            mfx_frame_allocator_.reset(new D3DFrameAllocator());
            D3DAllocatorParams* pd3dAllocParams = new D3DAllocatorParams;
            pd3dAllocParams->pManager = reinterpret_cast<IDirect3DDeviceManager9*>(hdl);
            allocator_params_.reset(pd3dAllocParams);
        }
        status = mfx_session_.SetFrameAllocator(mfx_frame_allocator_.get());
        external_alloc_ = true;
    } else {
        mfx_frame_allocator_.reset(new SysMemFrameAllocator());
    }
    status = mfx_frame_allocator_->Init(allocator_params_.get());
    return status;
}

mfxStatus VideoEncoderQSV::InitEnocdeParams() {
    // MFX_CODEC_HEVC
    encode_param_.mfx.CodecId = MFX_CODEC_AVC;
    encode_param_.mfx.GopOptFlag = MFX_GOP_CLOSED;
    encode_param_.mfx.NumSlice = 1;
    if (enable_hd_mode_) {
        encode_param_.mfx.TargetUsage = MFX_TARGETUSAGE_BEST_QUALITY;
        encode_param_.mfx.MaxKbps = 10 * 1024;
    } else {
        encode_param_.mfx.TargetUsage = MFX_TARGETUSAGE_BEST_SPEED;
        encode_param_.mfx.MaxKbps = 8 * 1024;
    }
    encode_param_.mfx.TargetKbps = 6 * 1024;
    encode_param_.mfx.FrameInfo.FrameRateExtN = frame_rate_;
    encode_param_.mfx.FrameInfo.FrameRateExtD = 1;
    encode_param_.mfx.FrameInfo.FourCC = MFX_FOURCC_NV12;
    encode_param_.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    encode_param_.mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
    encode_param_.mfx.FrameInfo.CropX = 0;
    encode_param_.mfx.FrameInfo.CropY = 0;
    encode_param_.mfx.FrameInfo.CropW = output_width_;
    encode_param_.mfx.FrameInfo.CropH = output_height_;
    encode_param_.mfx.RateControlMethod = MFX_RATECONTROL_VBR;
    encode_param_.AsyncDepth = 1;
    encode_param_.mfx.GopPicSize = UINT16_MAX;
    static mfxExtBuffer* extendedBuffers[1];
    encode_param_.mfx.NumRefFrame = 1;
    encode_param_.mfx.GopRefDist = 1;
    memset(&coding_opt_, 0, sizeof(mfxExtCodingOption));
    coding_opt_.Header.BufferId = MFX_EXTBUFF_CODING_OPTION;
    coding_opt_.Header.BufferSz = sizeof(mfxExtCodingOption);
    coding_opt_.MaxDecFrameBuffering = 1;
    extendedBuffers[0] = (mfxExtBuffer*)&coding_opt_;
    encode_param_.ExtParam = extendedBuffers;
    encode_param_.NumExtParam = 1;
    encode_param_.mfx.FrameInfo.Width = MSDK_ALIGN16(output_width_);
    encode_param_.mfx.FrameInfo.Height = MSDK_ALIGN16(output_height_);
    if (mem_type_ == kMemTypeSystem) {
        encode_param_.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;
    } else {
        encode_param_.IOPattern = MFX_IOPATTERN_IN_VIDEO_MEMORY;
    }
    return MFX_ERR_NONE;
}

// 分配 surface
mfxStatus VideoEncoderQSV::AllocFrames() {
    mfxFrameAllocRequest encode_request;
    MSDK_ZERO_MEMORY(encode_request);
    mfxStatus sts = mfx_encoder_->QueryIOSurf(&encode_param_, &encode_request);

    // alloc frames for encoder
    sts = mfx_frame_allocator_->Alloc(mfx_frame_allocator_->pthis, &encode_request,
                                      &encode_response_);
    surface_nums_ = encode_response_.NumFrameActual;
    frame_surfaces_.reset(new mfxFrameSurface1[surface_nums_]);
    for (int i = 0; i < surface_nums_; ++i) {
        memset(&(frame_surfaces_[i]), 0, sizeof(mfxFrameSurface1));
        MSDK_MEMCPY_VAR(frame_surfaces_[i].Info, &(encode_param_.mfx.FrameInfo),
                        sizeof(mfxFrameInfo));
        if (mem_type_ == kMemTypeSystem) {
            sts = mfx_frame_allocator_->Lock(mfx_frame_allocator_->pthis, encode_response_.mids[i],
                                             &(frame_surfaces_[i].Data));
        } else {
            frame_surfaces_[i].Data.MemId = encode_response_.mids[i];
        }
    }
    alloc_frame_ = true;
    return MFX_ERR_NONE;
}

mfxStatus VideoEncoderQSV::DeleteFrames() {
    if (mfx_frame_allocator_ && alloc_frame_) {
        mfx_frame_allocator_->Free(mfx_frame_allocator_->pthis, &encode_response_);
    }
    alloc_frame_ = false;
    return MFX_ERR_NONE;
}

mfxStatus VideoEncoderQSV::CreateHWDevice() {
    mfxStatus sts = MFX_ERR_NONE;
    if (kMemTypeD3D11 == mem_type_) {
        hw_device_.reset(new CD3D11Device());
    } else {
        hw_device_.reset(new CD3D9Device());
    }
    sts = hw_device_->Init(NULL, 0, MSDKAdapter::GetNumber(mfx_session_));
    return MFX_ERR_NONE;
}

mfxStatus VideoEncoderQSV::InitBitstream() {
    mfxVideoParam param = {0};
    mfxStatus sts = mfx_encoder_->GetVideoParam(&param);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
    memset(&output_bitstream_, 0, sizeof(mfxBitstream));
    output_bitstream_.MaxLength = param.mfx.BufferSizeInKB * 1000;
    output_bitstream_.DataOffset = 0;
    output_bitstream_.DataLength = 0;
    output_bitstream_.Data = nullptr;
    return MFX_ERR_NONE;
}