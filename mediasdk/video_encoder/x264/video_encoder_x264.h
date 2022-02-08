#pragma once
#include <vector>
#include "video_encoder.h"

#include "libx264/x264.h"

class VideoEncoderX264 : public VideoEncoder {
public:
    VideoEncoderX264();
    ~VideoEncoderX264();

    void EncodeFrame(std::shared_ptr<VideoFrame> video_frame, bool keyframe) override;

private:
    bool Init();
    bool Uninit();

private:
    x264_t* x264_encoder_{};
    x264_picture_t input_picture_;
    bool init_{};
    std::vector<uint8_t> buffer_{};
    uint32_t buffer_size_{};
};