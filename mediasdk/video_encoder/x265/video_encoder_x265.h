#pragma once
#include <vector>
#include "video_encoder.h"

#include "libx265/x265.h"

class VideoEncoderX265 : public VideoEncoder {
public:
    VideoEncoderX265();
    ~VideoEncoderX265();

    void EncodeFrame(std::shared_ptr<VideoFrame> video_frame, bool keyframe) override;

private:
    bool Init();
    bool Uninit();

private:
    bool init_{};
    std::vector<uint8_t> buffer_{};
    uint32_t buffer_size_{};

    x265_encoder* x265_encoder_{};
    x265_picture* input_picture_{};
};