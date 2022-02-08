#include "video_frame.h"

#include "common/buffer_pool.h"
#include <iostream>

VideoFrame::VideoFrame(uint32_t width, uint32_t height, FrameType frame_type, bool use_pool)
    : width_(width), height_(height), frame_type_(frame_type), use_pool_(use_pool) {
    switch (frame_type) {
    case kFrameTypeARGB:
        size_ = width * height * 4;
        break;
    case kFrameTypeI420:
    case kFrameTypeNV12:
        size_ = width * height * 3 / 2;
        break;
    default:
        break;
    }
    if (use_pool_) {
        BufferPool::GetInstance().GetBufferPool(size_).try_dequeue(buffer_);
        bool need_reset = false;
        if (!buffer_) {
            need_reset = true;
        }
        if (buffer_ && buffer_->GetSize() != size_) {
            need_reset = true;
        }
        if (need_reset) {
            buffer_.reset(new Buffer(size_));
        }
    } else {
        // buffer_.reset(new Buffer(size_));
        data_ = new uint8_t[size_];
    }
}

VideoFrame::~VideoFrame() {
    if (use_pool_) {
        BufferPool::GetInstance().GetBufferPool(size_).enqueue(buffer_);
    } else {
        if (data_) {
            delete[] data_;
        }
    }
}

void VideoFrame::SetPitch(uint32_t pitch) {
    pitch_ = pitch;
}

uint32_t VideoFrame::GetWidth() {
    return width_;
}

uint32_t VideoFrame::GetHeight() {
    return height_;
}

uint32_t VideoFrame::GetPitch() {
    switch (frame_type_) {
    case kFrameTypeARGB:
        return width_ * 4;
        break;
    case kFrameTypeI420:
    case kFrameTypeNV12:
        return width_;
    default:
        break;
    }
    return pitch_;
}

uint32_t VideoFrame::GetSize() {
    return size_;
}

uint8_t* VideoFrame::GetData() {
    if (use_pool_) {
        return buffer_->GetData();
    } else {
        return data_;
    }
}

FrameType VideoFrame::GetFrameType() {
    return frame_type_;
}