#include "video_encoder_ffmpeg.h"

#include "yuv/libyuv.h"

#include <iostream>

VideoEncoderFFmpeg::VideoEncoderFFmpeg() {}

VideoEncoderFFmpeg::~VideoEncoderFFmpeg() {}

void VideoEncoderFFmpeg::EncodeFrame(std::shared_ptr<VideoFrame> video_frame, bool keyframe) {
    uint32_t width = video_frame->GetWidth();
    uint32_t height = video_frame->GetHeight();
    if (!init_) {
        Init();
    }
    AVPixelFormat pix_fmt = AV_PIX_FMT_YUV420P;
    int size = avpicture_get_size(pix_fmt, output_width_, output_height_);
    bool need_scale = (width != output_width_) || (height != output_height_);
    uint8_t* src = video_frame->GetData();
    libyuv::I420Scale(src, width, src + width * height, width >> 1, src + width * height * 5 / 4,
                      width >> 1, width, height, (uint8_t*)frame_->data[0], frame_->linesize[0],
                      (uint8_t*)frame_->data[1], frame_->linesize[1], (uint8_t*)frame_->data[2],
                      frame_->linesize[2], output_width_, output_height_,
                      libyuv::FilterMode::kFilterBox);
    AVPacket packet;
    packet.data = NULL; // packet data will be allocated by the encoder
    packet.size = 0;
    av_init_packet(&packet);

    int got_packet = 0;
    int ret = avcodec_encode_video2(codec_context_, &packet, frame_, &got_packet);
    if (got_packet != 0) {
        if (callback_) {
            callback_(packet.data, packet.size);
        }
        av_packet_unref(&packet);
    }
}

bool VideoEncoderFFmpeg::Init() {
    av_register_all();
    avcodec_register_all();
    AVFormatContext* format_context = avformat_alloc_context();
    av_codec_ = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!av_codec_) {
        std::cout << "not found avcodec" << std::endl;
        return false;
    }
    codec_context_ = avcodec_alloc_context3(av_codec_);
    codec_context_->bit_rate = 300000; // 300kbps
    codec_context_->rc_max_rate = 300000;
    /* resolution must be a multiple of two */
    codec_context_->width = output_width_;
    codec_context_->height = output_height_;

    /* frames per second */
    codec_context_->time_base.den = 25;
    codec_context_->time_base.num = 1;
    // codec_context_->framerate.num = 25;
    // codec_context_->framerate.den = 1;

    codec_context_->gop_size = INT32_MAX; /* emit one intra frame every ten frames */
    codec_context_->max_b_frames = 0;
    codec_context_->pix_fmt = (enum AVPixelFormat)AV_PIX_FMT_YUV420P;
    codec_context_->qmax = 2;
    codec_context_->qmin = 32;
    codec_context_->delay = 0;

    AVDictionary* options = NULL;
    av_dict_set(&options, "preset", "medium", 0);
    av_dict_set(&options, "tune", "zerolatency", 0);
    av_dict_set(&options, "profile", "baseline", 0);
    avcodec_open2(codec_context_, av_codec_, &options);

    frame_ = av_frame_alloc();
    frame_->format = codec_context_->pix_fmt;
    frame_->width = codec_context_->width;
    frame_->height = codec_context_->height;
    av_image_alloc(frame_->data, frame_->linesize, frame_->width, frame_->height,
                   codec_context_->pix_fmt, 32);

    init_ = true;
    return true;
}

bool VideoEncoderFFmpeg::Uninit() {
    if (!init_) {
        return true;
    }
    avcodec_close(codec_context_);
    if (frame_) {
        av_frame_free(&frame_);
    }
    init_ = false;
    return true;
}