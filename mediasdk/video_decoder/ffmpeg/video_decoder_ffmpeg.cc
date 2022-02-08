#include "video_decoder_ffmpeg.h"

#include <iostream>
VideoDecocerFFmpeg::VideoDecocerFFmpeg() {
    InitDecoder();
}

VideoDecocerFFmpeg::~VideoDecocerFFmpeg() {
    UninitDecoder();
}

void VideoDecocerFFmpeg::Decode(uint8_t* data, uint32_t len) {
    packet_.size = len;
    packet_.data = data;
    int got_frame = 0;
    // avcodec_decode_video2: 在新版本中接口被废弃
    int frame_len = avcodec_decode_video2(codec_context_, frame_, &got_frame, &packet_);
    if (frame_len <= 0) {
        return;
    }
    std::shared_ptr<VideoFrame> video_frame(
        new VideoFrame(frame_->width, frame_->height, kFrameTypeI420, true));
    uint8_t* y_data = video_frame->GetData();
    uint8_t* u_data = y_data + frame_->width * frame_->height;
    uint8_t* v_data = y_data + frame_->width * frame_->height * 5 / 4;
    if (frame_->width == frame_->linesize[0]) {
        memcpy(y_data, frame_->data[0], frame_->width * frame_->height);
        memcpy(u_data, frame_->data[1], frame_->width * frame_->height / 4);
        memcpy(v_data, frame_->data[2], frame_->width * frame_->height / 4);
    }
    else {
        for (uint32_t i = 0; i < frame_->height; i++) {
            memcpy(y_data + i * frame_->width, frame_->data[0] + i * frame_->linesize[0],
                frame_->width);
        }
        for (uint32_t i = 0; i < frame_->height / 2; i++) {
            memcpy(u_data + i * frame_->width / 2, frame_->data[1] + i * frame_->linesize[1],
                frame_->width / 2);
        }
        for (uint32_t i = 0; i < frame_->height / 2; i++) {
            memcpy(v_data + i * frame_->width / 2, frame_->data[2] + i * frame_->linesize[2],
                frame_->width / 2);
        }
    }
    if (callback_) {
        callback_(video_frame);
    }
#if 0
    int ret = avcodec_send_packet(codec_context_, &packet_);
    if (ret < 0) {
        return;
    }
    while (ret >= 0) {
        ret = avcodec_receive_frame(codec_context_, frame_);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            break;
        }
        std::shared_ptr<VideoFrame> video_frame(
            new VideoFrame(frame_->width, frame_->height, kFrameTypeI420));
        uint8_t* y_data = video_frame->GetData();
        uint8_t* u_data = y_data + frame_->width * frame_->height;
        uint8_t* v_data = y_data + frame_->width * frame_->height * 5 / 4;
        if (frame_->width == frame_->linesize[0]) {
            memcpy(y_data, frame_->data[0], frame_->width * frame_->height);
            memcpy(u_data, frame_->data[1], frame_->width * frame_->height / 4);
            memcpy(v_data, frame_->data[2], frame_->width * frame_->height / 4);
        }
        else {
            for (uint32_t i = 0; i < frame_->height; i++) {
                memcpy(y_data + i * frame_->width, frame_->data[0] + i * frame_->linesize[0],
                    frame_->width);
            }
            for (uint32_t i = 0; i < frame_->height / 2; i++) {
                memcpy(u_data + i * frame_->width / 2, frame_->data[1] + i * frame_->linesize[1],
                    frame_->width / 2);
            }
            for (uint32_t i = 0; i < frame_->height / 2; i++) {
                memcpy(v_data + i * frame_->width / 2, frame_->data[2] + i * frame_->linesize[2],
                    frame_->width / 2);
            }
        }
        if (callback_) {
            callback_(video_frame);
        }
    }
#endif
}

bool VideoDecocerFFmpeg::InitDecoder() {
    avcodec_register_all();
    codec_ = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec_) {
        return false;
    }
    codec_context_ = avcodec_alloc_context3(codec_);
    if (!codec_context_) {
        return false;
    }
    // codec_context_->max_pixels = 3840 * 2160;
    int ret = avcodec_open2(codec_context_, codec_, NULL);
    if (ret < 0) {
        return false;
    }
    frame_ = av_frame_alloc();
    if (!frame_) {
        return false;
    }
    av_init_packet(&packet_);
    return true;
}

bool VideoDecocerFFmpeg::UninitDecoder() {
    avcodec_close(codec_context_);
    avcodec_free_context(&codec_context_);
    av_frame_free(&frame_);
    return true;
}