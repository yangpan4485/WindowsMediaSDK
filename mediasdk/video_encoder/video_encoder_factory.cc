#include "video_encoder_factory.h"

// x264 和 openh264 头文件会有冲突
#include "x264/video_encoder_x264.h"
#include "x265/video_encoder_x265.h"
// #include "openh264/video_encoder_openh264.h"
#include "ffmpeg/video_encoder_ffmpeg.h"
#include "qsv/video_encoder_qsv.h"

VideoEnocderFcatory& VideoEnocderFcatory::Instance() {
    static VideoEnocderFcatory instance;
    return instance;
}

std::shared_ptr<VideoEncoder> VideoEnocderFcatory::CreateEncoder(EncodeType encode_type) {
    std::shared_ptr<VideoEncoder> video_encoder = nullptr;
    switch (encode_type)
    {
    case kEncodeTypeX264:
        video_encoder.reset(new VideoEncoderX264());
        break;
    case kEncodeTypeX265:
        video_encoder.reset(new VideoEncoderX265());
        break;
    case kEncodeTypeOpenH264:
        video_encoder.reset(new VideoEncoderX264());
        // video_encoder.reset(new VideoEncoderOpenH264());
        break;
    case kEncodeTypeFFmpeg:
        video_encoder.reset(new VideoEncoderFFmpeg());
        break;
    case kEncodeTypeQSV:
        video_encoder.reset(new VideoEncoderQSV());
        break;
    default:
        break;
    }
    return video_encoder;
}

VideoEnocderFcatory::VideoEnocderFcatory() {
}
VideoEnocderFcatory::~VideoEnocderFcatory() {

}