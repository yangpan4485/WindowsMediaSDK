#include "video_encoder_x265.h"

#include "yuv/libyuv.h"

VideoEncoderX265::VideoEncoderX265() {}

VideoEncoderX265::~VideoEncoderX265() {}

void VideoEncoderX265::EncodeFrame(std::shared_ptr<VideoFrame> video_frame, bool keyframe) {
    uint32_t width = video_frame->GetWidth();
    uint32_t height = video_frame->GetHeight();
    if (!init_) {
        if (!Init()) {
            return;
        }
    }
    x265_nal* nal = nullptr;
    x265_picture pic_out;
    uint32_t i_nal = 0;
    input_picture_->sliceType = keyframe ? X265_TYPE_IDR : X265_TYPE_AUTO;
    bool need_scale = (width != output_width_) || (height != output_height_);
    uint8_t* src = video_frame->GetData();
    libyuv::I420Scale(src, width, src + width * height, width >> 1, src + width * height * 5 / 4,
                      width >> 1, width, height, (uint8_t*)input_picture_->planes[0],
                      input_picture_->stride[0], (uint8_t*)input_picture_->planes[1],
                      input_picture_->stride[1], (uint8_t*)input_picture_->planes[2],
                      input_picture_->stride[2], output_width_, output_height_,
                      libyuv::FilterMode::kFilterBox);
    int framesize = x265_encoder_encode(x265_encoder_, &nal, &i_nal, input_picture_, NULL);
    if (callback_ && framesize > 0) {
        for (int i = 0; i < i_nal; ++i) {
            callback_(nal[i].payload, nal[i].sizeBytes);
        }
    }
}

bool VideoEncoderX265::Init() {
    x265_param param;
    uint32_t bitrate = 6 * 1024;
    if (enable_hd_mode_) {
        x265_param_default_preset(&param, "veryfast", "zerolatency");
    } else {
        x265_param_default_preset(&param, "faster", "zerolatency");
    }

    param.sourceWidth = output_width_;
    param.sourceHeight = output_height_;
    // 一定要加这个
    param.bRepeatHeaders = 1;
    param.fpsNum = frame_rate_;
    param.fpsDenom = 1;
    param.frameNumThreads = 1;
    param.logLevel = X265_LOG_WARNING;
    param.internalCsp = X265_CSP_I420;
    param.internalBitDepth = 8;
    param.bframes = 0;
    param.keyframeMax = -1;

    param.rc.rateControlMode = X265_RC_ABR;
    param.rc.bitrate = (int)bitrate;
    param.rc.vbvMaxBitrate = param.rc.bitrate * 1.1;
    if (enable_hd_mode_) {
        param.rc.vbvBufferSize = param.rc.bitrate * 1.1;
        param.rc.qpMax = 34;
    } else {
        param.rc.vbvBufferSize = param.rc.bitrate * 0.8;
        param.rc.qpMax = 39;
    }

    input_picture_ = x265_picture_alloc();
    x265_picture_init(&param, input_picture_);
    input_picture_->bitDepth = 8;
    input_picture_->colorSpace = X265_CSP_I420;
    input_picture_->stride[0] = param.sourceWidth;
    input_picture_->stride[1] =
        input_picture_->stride[0] >> x265_cli_csps[input_picture_->colorSpace].width[1];
    input_picture_->stride[2] =
        input_picture_->stride[0] >> x265_cli_csps[input_picture_->colorSpace].width[2];
    int y_size = param.sourceWidth * param.sourceHeight;
    input_picture_->planes[0] = (char*)malloc(y_size);
    input_picture_->planes[1] = (char*)malloc(y_size / 4);
    input_picture_->planes[2] = (char*)malloc(y_size / 4);

    x265_encoder_ = x265_encoder_open(&param);
    init_ = true;
    return true;
}

bool VideoEncoderX265::Uninit() {
    if (init_) {
        if (input_picture_->planes[0]) {
            free(input_picture_->planes[0]);
        }
        if (input_picture_->planes[1]) {
            free(input_picture_->planes[1]);
        }
        if (input_picture_->planes[2]) {
            free(input_picture_->planes[2]);
        }
        x265_picture_free(input_picture_);
        x265_encoder_close(x265_encoder_);
        x265_encoder_ = nullptr;
        x265_cleanup();
        init_ = false;
        return 0;
    }
    return true;
}
