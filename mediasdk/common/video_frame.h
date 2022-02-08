#pragma once
#include <cstdint>
#include <memory>

enum FrameType {
    kFrameTypeARGB = 0,
    kFrameTypeI420 = 1,
    kFrameTypeNV12 = 2,
};

class Buffer;
class VideoFrame {
public:
    VideoFrame(uint32_t width, uint32_t height, FrameType frame_type, bool use_pool);
    ~VideoFrame();

    void SetPitch(uint32_t pitch);
    uint32_t GetWidth();
    uint32_t GetHeight();
    uint32_t GetPitch();
    uint32_t GetSize();
    uint8_t* GetData();
    FrameType GetFrameType();

private:
    uint32_t width_{};
    uint32_t height_{};
    uint32_t pitch_{};
    uint8_t* data_{};
    FrameType frame_type_{};
    std::shared_ptr<Buffer> buffer_{};
    uint32_t size_{};
    bool use_pool_{false};
};