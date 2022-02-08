#include "video_encoder.h"

VideoEncoder::VideoEncoder() {

}

VideoEncoder::~VideoEncoder() {

}

void VideoEncoder::EncodeFrame(std::shared_ptr<VideoFrame> video_frame, bool keyframe) {

}

void VideoEncoder::RegisterEncodeCalback(EncodeFrameCallback callback) {
    callback_ = callback;
}

void VideoEncoder::SetOutputSize(uint32_t width, uint32_t height) {
    /*output_width_ = width % 16 == 0 ? width : width + (16 - width % 16);
    output_height_ = height % 16 == 0 ? height : height + (16 - height % 16);*/
    output_width_ = width;
    output_height_ = height;
}