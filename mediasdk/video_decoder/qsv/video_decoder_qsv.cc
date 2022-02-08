#include "video_decoder_qsv.h"
#include <cstdio>
#include <iostream>
#include <vector>

#include "video_frame.h"

VideoDecoderQSV::VideoDecoderQSV() : stream_buffers_(2) {
    impl_ = MFX_IMPL_HARDWARE_ANY | MFX_IMPL_VIA_D3D11;
    Init();
}

VideoDecoderQSV::~VideoDecoderQSV() {
    Stop();
    Uninit();
}

void VideoDecoderQSV::Decode(uint8_t* data, uint32_t len) {
    // 把数据放到 buffer 里面，然后从 buffer 里面去读，然后解码
    auto buffer = std::vector<uint8_t>(data, data + len);
    stream_buffers_.enqueue(std::move(buffer));
    if (!running_) {
        Start();
    }
}

void VideoDecoderQSV::Stop() {
    std::unique_lock<std::mutex> lock(mtx_);
    if (!running_) {
        return;
    }
    running_ = false;
    if (decode_thread_.joinable()) {
        decode_thread_.join();
    }
    stream_buffers_.stop_blocking();
    mfx_decoder_->Close();
    ReleaseFrames();
}

void VideoDecoderQSV::Start() {
    std::unique_lock<std::mutex> lock(mtx_);
    if (running_) {
        return;
    }
    running_ = true;
    decode_thread_ = std::thread([&]() {
        std::vector<uint8_t> buffer;
        std::vector<uint8_t> stream_buffer;
        while (running_) {
            // 30ms
            bool ret = stream_buffers_.wait_dequeue_timed(buffer, 30000);
            if (!ret) {
                // 重复渲染上一帧
                continue;
            }
            if (!init_decoder_) {
                stream_buffer.insert(stream_buffer.end(), buffer.begin(), buffer.end());
                if (!ResetDecoder(stream_buffer)) {
                    std::cout << "ResetDecoder false" << std::endl;
                    continue;
                }
                std::cout << "ResetDecoder success" << std::endl;
            }
            mfxStatus sts = MFX_ERR_NONE;
            mfxSyncPoint syncp = nullptr;
            mfxBitstream bitstream = {};
            bitstream.DataFlag = MFX_BITSTREAM_COMPLETE_FRAME;
            if (stream_buffer.size() > 0) {
                // first
                bitstream.Data = stream_buffer.data();
                bitstream.DataLength = stream_buffer.size();
                bitstream.MaxLength = stream_buffer.size();
                stream_buffer.clear();
            } else {
                bitstream.Data = buffer.data();
                bitstream.DataLength = buffer.size();
                bitstream.MaxLength = buffer.size();
            }
            mfxU16 free_surface_index = GetFreeSurfaceIndex(decode_surface_.get(), surface_nums_);
            mfxFrameSurface1 surface = decode_surface_[free_surface_index];
            while (running_) {
                sts = mfx_decoder_->DecodeFrameAsync(&bitstream, &surface, &out_surface_, &syncp);
                if (MFX_ERR_NONE < sts && !syncp) {
                    if (MFX_WRN_DEVICE_BUSY == sts) {
                        MSDK_SLEEP(1);
                    }
                } else if (MFX_ERR_NONE < sts && syncp) {
                    sts = MFX_ERR_NONE; // Ignore warnings if output is
                    break;
                } else {
                    break;
                }
            }
            if (sts == MFX_ERR_INCOMPATIBLE_VIDEO_PARAM) {
                std::cout << "reinit" << std::endl;
                init_decoder_ = false;
                continue;
            } else if (sts != MFX_ERR_NONE) {
                continue;
            }
            while (running_) {
                sts = mfx_session_.SyncOperation(syncp, 50000);
                if (sts != MFX_ERR_NONE) {
                    continue;
                }
                if (need_fetch_info_) {
                    GetVideoSignalInfo();
                    need_fetch_info_ = false;
                }
                if (render_window_) {
                    RECT rect;
                    hw_device_->RenderFrame(out_surface_, frame_allocator_.get(),
                                            &video_signal_info_, true, &rect);
                } else if (callback_) {
                    std::shared_ptr<VideoFrame> video_frame(new VideoFrame(
                        out_surface_->Info.Width, out_surface_->Info.Height, kFrameTypeNV12, true));
                    uint8_t* y_data = video_frame->GetData();
                    uint8_t* uv_data =
                        y_data + out_surface_->Info.Width * out_surface_->Info.Height;
                    if (out_surface_->Data.Pitch == out_surface_->Info.Width) {
                        memcpy(y_data, out_surface_->Data.Y,
                               out_surface_->Info.Width * out_surface_->Info.Height);
                        memcpy(uv_data, out_surface_->Data.UV,
                               out_surface_->Info.Width * out_surface_->Info.Height / 2);
                    } else {
                        for (uint32_t i = 0; i < out_surface_->Info.Height; i++) {
                            memcpy(y_data + i * out_surface_->Info.Width,
                                   out_surface_->Data.Y + i * out_surface_->Data.Pitch,
                                   out_surface_->Info.Width);
                        }
                        for (uint32_t i = 0; i < out_surface_->Info.Height / 2; i++) {
                            memcpy(uv_data + i * out_surface_->Info.Width,
                                   out_surface_->Data.UV + i * out_surface_->Data.Pitch,
                                   out_surface_->Info.Width);
                        }
                    }
                    callback_(video_frame);
                }
            }
        }
    });
}

void VideoDecoderQSV::SetRenderHwnd(HWND hwnd) {
    render_window_ = hwnd;
    if (hw_device_) {
        hw_device_->SetHandle((mfxHandleType)MFX_HANDLE_DEVICEWINDOW, render_window_);
    }
}

bool VideoDecoderQSV::Init() {
    mfxVersion version = {{0, 1}};
    mfxStatus status = mfx_session_.Init(impl_, &version);
    mfx_session_.QueryIMPL(&impl_);
    if (status != MFX_ERR_NONE) {
        return false;
    }
    mfx_decoder_.reset(new MFXVideoDECODE(mfx_session_));
    bool enable_hevc = false;
    if (enable_hevc) {
        video_param_.mfx.CodecId = MFX_CODEC_HEVC;
        mfxPluginUID uid = MFX_PLUGINID_HEVCD_HW;
        status = MFXVideoUSER_Load(mfx_session_, &uid, version.Major);
        if (status != MFX_ERR_NONE) {
            return false;
        }
    }
    video_param_.mfx.CodecId = MFX_CODEC_AVC;
    CreateHWDevice();
    CreateAllocator();
    return true;
}

bool VideoDecoderQSV::Uninit() {
    mfx_decoder_->Close();
    mfx_session_.Close();
    ReleaseFrames();
    return true;
}

bool VideoDecoderQSV::ResetDecoder(std::vector<uint8_t>& stream) {
    std::cout << "stream size: " << stream.size() << std::endl;
    mfxBitstream bit_stream;
    bit_stream.Data = stream.data();
    bit_stream.DataLength = stream.size();
    bit_stream.MaxLength = stream.size();
    mfxStatus status = mfx_decoder_->DecodeHeader(&bit_stream, &video_param_);
    if (status != MFX_ERR_NONE) {
        std::cout << "DecodeHeader false: " << status << std::endl;
        return false;
    }
    mfx_decoder_->Close();
    InitDecoderParams();
    ReleaseFrames();
    AllocFrames();
    status = mfx_decoder_->Init(&video_param_);
    init_decoder_ = true;
    need_fetch_info_ = true;
    return true;
}

bool VideoDecoderQSV::InitDecoderParams() {
    video_param_.AsyncDepth = 1;
    video_param_.IOPattern = MFX_IOPATTERN_OUT_VIDEO_MEMORY;
    video_param_.mfx.FrameInfo.FourCC = MFX_FOURCC_NV12;
    video_param_.mfx.FrameInfo.FrameRateExtN = 60;
    video_param_.mfx.FrameInfo.FrameRateExtD = 1;
    video_param_.mfx.FrameInfo.AspectRatioW = 1;
    video_param_.mfx.FrameInfo.AspectRatioH = 1;
    video_param_.mfx.DecodedOrder = 1;
    return true;
}

bool VideoDecoderQSV::CreateAllocator() {
    frame_allocator_.reset(new GeneralAllocator());
    mfxHDL hdl = NULL;
    mfxHandleType hdl_t = MFX_HANDLE_D3D11_DEVICE;
    mfxStatus status = hw_device_->GetHandle(hdl_t, &hdl);
    mfx_session_.QueryIMPL(&impl_);
    status = mfx_session_.SetHandle(hdl_t, hdl);
    mfx_frame_allocator_.reset(new D3D11FrameAllocator());
    D3DAllocatorParams* pd3dAllocParams = new D3DAllocatorParams;
    pd3dAllocParams->pManager = reinterpret_cast<IDirect3DDeviceManager9*>(hdl);
    allocator_params_.reset(pd3dAllocParams);
    status = mfx_session_.SetFrameAllocator(mfx_frame_allocator_.get());
    status = mfx_frame_allocator_->Init(allocator_params_.get());
    return true;
}

bool VideoDecoderQSV::CreateHWDevice() {
    hw_device_.reset(new CD3D11Device());
    mfxStatus sts = hw_device_->Init(NULL, 0, MSDKAdapter::GetNumber(mfx_session_));
    return sts == MFX_ERR_NONE;
}

bool VideoDecoderQSV::AllocFrames() {
    mfxFrameAllocRequest decode_request;
    MSDK_ZERO_MEMORY(decode_request);
    mfxStatus sts = mfx_decoder_->Query(&video_param_, &video_param_);
    sts = mfx_decoder_->QueryIOSurf(&video_param_, &decode_request);
    decode_request.Type = MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET;
    sts =
        mfx_frame_allocator_->Alloc(mfx_frame_allocator_->pthis, &decode_request, &alloc_response_);
    surface_nums_ = alloc_response_.NumFrameActual;
    std::cout << "surface_nums_: " << surface_nums_ << std::endl;
    decode_surface_.reset(new mfxFrameSurface1[surface_nums_]);
    for (int i = 0; i < surface_nums_; ++i) {
        memset(&(decode_surface_[i]), 0, sizeof(mfxFrameSurface1));
        MSDK_MEMCPY_VAR(decode_surface_[i].Info, &(video_param_.mfx.FrameInfo),
                        sizeof(mfxFrameInfo));
        decode_surface_[i].Data.MemId = alloc_response_.mids[i];
    }
    return true;
}

bool VideoDecoderQSV::ReleaseFrames() {
    frame_allocator_->FreeFrames(&alloc_response_);
    return true;
}

void VideoDecoderQSV::GetVideoSignalInfo() {
    mfxVideoParam videoParam = {};
    video_signal_info_.Header.BufferId = MFX_EXTBUFF_VIDEO_SIGNAL_INFO;
    video_signal_info_.Header.BufferSz = sizeof(mfxExtVideoSignalInfo);
    mfxExtBuffer* ext_param[] = {(mfxExtBuffer*)&video_signal_info_};
    videoParam.ExtParam = ext_param;
    videoParam.NumExtParam = 1;
    mfx_decoder_->GetVideoParam(&videoParam);
}