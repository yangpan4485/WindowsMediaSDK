#include "video_encoder_openh264.h"

#include "yuv/libyuv.h"

#include <iostream>

VideoEncoderOpenH264::VideoEncoderOpenH264() {
    encode_param_ = new SEncParamExt;
    picture_ = new SSourcePicture;
}

VideoEncoderOpenH264::~VideoEncoderOpenH264() {
    Uninit();
    if (encode_param_) {
        delete encode_param_;
        encode_param_ = nullptr;
    }
    if (picture_) {
        delete picture_;
        picture_ = nullptr;
    }
}

void VideoEncoderOpenH264::EncodeFrame(std::shared_ptr<VideoFrame> video_frame, bool keyframe) {
    int width = video_frame->GetWidth();
    int height = video_frame->GetHeight();
    if (!init_) {
        Init();
    }
    SFrameBSInfo encoded_frame_info;
    bool need_scale = (width != output_width_) || (height != output_height_);
    uint8_t* src = video_frame->GetData();
    libyuv::I420Scale(src, width, src + width * height, width >> 1, src + width * height * 5 / 4,
                      width >> 1, width, height, (uint8_t*)picture_->pData[0], picture_->iStride[0],
                      (uint8_t*)picture_->pData[1], picture_->iStride[1],
                      (uint8_t*)picture_->pData[2], picture_->iStride[2], output_width_,
                      output_height_, libyuv::FilterMode::kFilterBox);
    int err = encoder_->EncodeFrame(picture_, &encoded_frame_info);
    if (encoded_frame_info.eFrameType == videoFrameTypeInvalid) {
        return;
    }
    if (encoded_frame_info.eFrameType != videoFrameTypeSkip) {
        int layer = 0;
        while (layer < encoded_frame_info.iLayerNum) {
            SLayerBSInfo* info = &(encoded_frame_info.sLayerInfo[layer]);
            if (info != NULL) {
                int size = 0;
                int nal_index = info->iNalCount - 1;
                do {
                    size += info->pNalLengthInByte[nal_index];
                    --nal_index;
                } while (nal_index >= 0);
                if (callback_) {
                    callback_(info->pBsBuf, size);
                }
            }
            ++layer;
        }
    }
}

bool VideoEncoderOpenH264::Init() {
    // 创建编码器对象
    uint32_t bitrate = 6 * 1024;
    int err = WelsCreateSVCEncoder(&encoder_);
    // 获取默认参数
    encoder_->GetDefaultParams(encode_param_);
    // 复杂度
    ECOMPLEXITY_MODE complexityMode = HIGH_COMPLEXITY;
    // 码控模式
    RC_MODES rc_mode = RC_BITRATE_MODE;
    // 其他的参数：分辨率、码率、帧率等等
    encode_param_->iPicWidth = output_width_;
    encode_param_->iPicHeight = output_height_;
    encode_param_->iTargetBitrate = bitrate;
    encode_param_->iMaxBitrate = bitrate;
    encode_param_->bEnableAdaptiveQuant = false;
    encode_param_->iRCMode = rc_mode;
    encode_param_->fMaxFrameRate = frame_rate_;
    encode_param_->iComplexityMode = complexityMode;
    encode_param_->iNumRefFrame = -1;
    encode_param_->eSpsPpsIdStrategy = CONSTANT_ID;
    encode_param_->iEntropyCodingModeFlag = 0; // 1;
    // encode_param_->bEnablePsnr = false;
    encode_param_->bEnableSSEI = true;
    encode_param_->bEnableSceneChangeDetect = true;
    // 设置QP，可以根据自己的需要来，QP越大码率越小（图像的质量越差）
    encode_param_->iMaxQp = 40;
    encode_param_->iMinQp = 30;
    encode_param_->uiIntraPeriod = UINT32_MAX;
    encode_param_->bEnableFrameSkip = false;
    encoder_->InitializeExt(encode_param_);

    memset(picture_, 0, sizeof(SSourcePicture));
    picture_->iPicWidth = output_width_;
    picture_->iPicHeight = output_height_;

    picture_->iColorFormat = videoFormatI420;
    picture_->iStride[0] = picture_->iPicWidth;
    picture_->iStride[1] = picture_->iStride[2] = picture_->iPicWidth >> 1;
    picture_buffer_ = new uint8_t[picture_->iPicWidth * picture_->iPicHeight * 3 / 2];
    picture_->pData[0] = (unsigned char*)picture_buffer_;
    picture_->pData[1] = picture_buffer_ + picture_->iPicWidth * picture_->iPicHeight;
    picture_->pData[2] = picture_buffer_ + (picture_->iPicWidth * picture_->iPicHeight * 5 / 4);

    init_ = true;
    return true;
}

bool VideoEncoderOpenH264::Uninit() {

    if (encoder_) {
        encoder_->Uninitialize();
        WelsDestroySVCEncoder(encoder_);
        encoder_ = NULL;
    }
    if (picture_buffer_) {
        delete[] picture_buffer_;
        picture_buffer_ = nullptr;
    }
    init_ = false;
    return true;
}