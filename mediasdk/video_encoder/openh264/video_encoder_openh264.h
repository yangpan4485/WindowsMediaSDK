#pragma once

#include <vector>
#include "video_encoder.h"

#include "openh264/wels/codec_api.h"

class VideoEncoderOpenH264 : public VideoEncoder {
public:
    VideoEncoderOpenH264();
    ~VideoEncoderOpenH264();

    void EncodeFrame(std::shared_ptr<VideoFrame> video_frame, bool keyframe) override;

private:
    bool Init();
    bool Uninit();

private:
    bool init_{};
    std::vector<uint8_t> buffer_{};
    uint32_t buffer_size_{};

    ISVCEncoder* encoder_{};
    TagEncParamExt* encode_param_{};
    Source_Picture_s* picture_{};
    uint8_t* picture_buffer_{};
};