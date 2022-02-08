#pragma once
#include <memory>
#include "video_decoder.h"

class VideoDecoderFactory {
public:
    static VideoDecoderFactory& GetInstance();
    std::shared_ptr<VideoDecoder> CreateVideoDecoder();

private:
    VideoDecoderFactory();
    ~VideoDecoderFactory();
    VideoDecoderFactory(const VideoDecoderFactory&) = delete;
    VideoDecoderFactory operator=(const VideoDecoderFactory&) = delete;
};