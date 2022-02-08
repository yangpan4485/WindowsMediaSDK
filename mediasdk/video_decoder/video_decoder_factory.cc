#include "video_decoder_factory.h"

#include "ffmpeg/video_decoder_ffmpeg.h"
#include "qsv/video_decoder_qsv.h"

VideoDecoderFactory::VideoDecoderFactory() {}

VideoDecoderFactory::~VideoDecoderFactory() {}

VideoDecoderFactory& VideoDecoderFactory::GetInstance() {
    static VideoDecoderFactory instance;
    return instance;
}

std::shared_ptr<VideoDecoder> VideoDecoderFactory::CreateVideoDecoder() {
    // QSV 有点问题，还在调试中，暂时不可用
    // std::shared_ptr<VideoDecoder> video_decoder(new VideoDecoderQSV());
    std::shared_ptr<VideoDecoder> video_decoder(new VideoDecocerFFmpeg());
    return video_decoder;
}