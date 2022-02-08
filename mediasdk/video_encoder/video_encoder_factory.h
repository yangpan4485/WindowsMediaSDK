#pragma once
#include <memory>
#include "video_encoder.h"

enum EncodeType {
    kEncodeTypeX264,
    kEncodeTypeX265,
    kEncodeTypeOpenH264,
    kEncodeTypeFFmpeg,
    kEncodeTypeQSV,
};

class VideoEnocderFcatory {
public:
    static VideoEnocderFcatory& Instance();
    std::shared_ptr<VideoEncoder> CreateEncoder(EncodeType encode_type);

private:
    VideoEnocderFcatory();
    ~VideoEnocderFcatory();

    VideoEnocderFcatory(const VideoEnocderFcatory&) = delete;
    VideoEnocderFcatory operator=(const VideoEnocderFcatory&) = delete;
};