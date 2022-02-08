#pragma once
#include <functional>
#include <fstream>
#include <vector>
#include "video_frame.h"

class VideoEncoder {
public:
    using EncodeFrameCallback = std::function<void(uint8_t* data, uint32_t len)>;
public:
    VideoEncoder();
    virtual ~VideoEncoder();

    virtual void EncodeFrame(std::shared_ptr<VideoFrame> video_frame, bool keyframe);

    void RegisterEncodeCalback(EncodeFrameCallback callback);
    void SetOutputSize(uint32_t width, uint32_t height);

protected:
    EncodeFrameCallback callback_{};
    uint32_t output_width_{1920};
    uint32_t output_height_{1080};
    /*uint32_t frame_width_{};
    uint32_t frame_height_{};*/
    uint32_t frame_rate_{10};
    bool enable_hd_mode_{};
    std::ofstream capture_fout_{};
    std::vector<uint8_t*> buffer_{};
};