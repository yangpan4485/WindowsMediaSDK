#pragma once
#include <cstdint>
#include "video_decoder.h"

extern "C" {
#include "libavcodec/avcodec.h"
// #include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
// #include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
// #include <libavformat/avformat.h>
};

class VideoDecocerFFmpeg : public VideoDecoder {
public:
    VideoDecocerFFmpeg();
    ~VideoDecocerFFmpeg();

    void Decode(uint8_t* data, uint32_t len) override;

private:
    bool InitDecoder();
    bool UninitDecoder();

private:
    AVCodec* codec_{};
    AVCodecContext* codec_context_{};
    AVFrame* frame_{};
    AVPacket packet_;
};