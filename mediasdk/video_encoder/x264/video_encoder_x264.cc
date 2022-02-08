#include "video_encoder_x264.h"

#include <vector>

#include "yuv/libyuv.h"

VideoEncoderX264::VideoEncoderX264() {}

VideoEncoderX264::~VideoEncoderX264() {
    Uninit();
}

void VideoEncoderX264::EncodeFrame(std::shared_ptr<VideoFrame> video_frame, bool keyframe) {
    uint32_t width = video_frame->GetWidth();
    uint32_t height = video_frame->GetHeight();
    if (!init_) {
        Init();
    }
    // x264 默认第一帧编码成关键帧
    x264_nal_t* nal;
    x264_picture_t pic_out;
    int i_nal;
    input_picture_.i_type = keyframe ? X264_TYPE_IDR : X264_TYPE_AUTO;
    bool need_scale = (width != output_width_) || (height != output_height_);
    uint8_t* src = video_frame->GetData();
    if (need_scale) {
        if (buffer_size_ != width * height * 3 / 2) {
            buffer_size_ = width * height * 3 / 2;
            buffer_.resize(buffer_size_);
        }
        uint8_t* dst = &buffer_[0];
        libyuv::I420Scale(src, width, src + width * height, width >> 1,
                          src + width * height * 5 / 4, width >> 1, width, height, dst,
                          output_width_, dst + output_width_ * output_height_, output_width_ >> 1,
                          dst + output_width_ * output_height_ * 5 / 4, output_width_ >> 1,
                          output_width_, output_height_, libyuv::FilterMode::kFilterBox);
        libyuv::I420ToNV12(dst, output_width_, dst + output_width_ * output_height_,
                           output_width_ >> 1, dst + output_width_ * output_height_ * 5 / 4,
                           output_width_ >> 1, input_picture_.img.plane[0],
                           input_picture_.img.i_stride[0], input_picture_.img.plane[1],
                           input_picture_.img.i_stride[1], output_width_, output_height_);
    } else {
        libyuv::I420ToNV12(src, output_width_, src + output_width_ * output_height_,
                           output_width_ >> 1, src + output_width_ * output_height_ * 5 / 4,
                           output_width_ >> 1, input_picture_.img.plane[0],
                           input_picture_.img.i_stride[0], input_picture_.img.plane[1],
                           input_picture_.img.i_stride[1], output_width_, output_height_);
    }
    int i_framesize = x264_encoder_encode(x264_encoder_, &nal, &i_nal, &input_picture_, &pic_out);
    if (callback_ && i_framesize > 0) {
        callback_(nal[0].p_payload, i_framesize);
    }
}

bool VideoEncoderX264::Uninit() {
    if (init_) {
        x264_encoder_close(x264_encoder_);
        x264_encoder_ = nullptr;
        x264_picture_clean(&input_picture_);
        init_ = false;
    }
    return true;
}

bool VideoEncoderX264::Init() {
    x264_param_t x264_param;
    uint32_t bitrate = 6 * 1024; // 6M bps
    if (enable_hd_mode_) {
        x264_param_default_preset(&x264_param, "veryfast", "zerolatency");
        // bitrate = 3600 + output_width_ * output_height_ / 400;
    } else {
        x264_param_default_preset(&x264_param, "ultrafast", "zerolatency");
        // bitrate = 3000 + output_width_ * output_height_ / 432;
    }
    x264_param.i_width = output_width_;
    x264_param.i_height = output_height_;
    x264_param.i_fps_num = frame_rate_;
    x264_param.i_csp = X264_CSP_NV12;
    x264_param.i_threads = 1;
    x264_param.i_keyint_max = X264_KEYINT_MAX_INFINITE;
    x264_param.i_log_level = X264_LOG_WARNING;
    x264_param.rc.i_rc_method = X264_RC_ABR;
    x264_param.rc.b_filler = 0;
    x264_param.rc.i_bitrate = (int)bitrate;
    x264_param.rc.i_vbv_max_bitrate = x264_param.rc.i_bitrate * 1.1;
    if (enable_hd_mode_) {
        x264_param.rc.i_vbv_buffer_size = x264_param.rc.i_bitrate * 1.1;
        x264_param.rc.i_qp_max = 40;
    } else {
        x264_param.rc.i_vbv_buffer_size = x264_param.rc.i_bitrate * 0.8;
        x264_param.rc.i_qp_max = 45;
    }
    x264_param.b_opencl = 0;
    x264_encoder_ = x264_encoder_open(&x264_param);
    x264_picture_alloc(&input_picture_, X264_CSP_NV12, output_width_, output_height_);
    init_ = true;
    return true;
}