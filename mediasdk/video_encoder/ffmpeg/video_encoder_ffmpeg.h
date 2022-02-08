#pragma once
#include <vector>
#include "video_encoder.h"

extern "C"
{
#include "libavutil\opt.h"
#include "libavcodec\avcodec.h"
#include "libavformat\avformat.h"
#include "libswscale\swscale.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
};

class VideoEncoderFFmpeg : public VideoEncoder {
public:
    VideoEncoderFFmpeg();
    ~VideoEncoderFFmpeg();

    void EncodeFrame(std::shared_ptr<VideoFrame> video_frame, bool keyframe) override;

private:
    bool Init();
    bool Uninit();

private:
    bool init_{};
    std::vector<uint8_t> buffer_{};
    uint32_t buffer_size_{};
    AVCodec* av_codec_{};
    AVCodecContext* codec_context_{};
    AVFrame* frame_{};
};