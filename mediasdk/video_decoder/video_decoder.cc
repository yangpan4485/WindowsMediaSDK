#include "video_decoder.h"

VideoDecoder::VideoDecoder() {}

VideoDecoder::~VideoDecoder() {}

void VideoDecoder::Decode(uint8_t* data, uint32_t len) {}

void VideoDecoder::SetRenderHwnd(HWND hwnd) {}

void VideoDecoder::SetDevoceFrameCallback(DevoceFrameCallback callback) {
    callback_ = callback;
}