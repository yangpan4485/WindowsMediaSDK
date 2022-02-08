#pragma once
#include <cstdint>
#include <string>

const uint32_t kMaxDeviceLength = 128;
#define RELEASE_AND_CLEAR(p) \
    if (p) {                 \
        (p)->Release();      \
        (p) = NULL;          \
    }

enum VideoType {
    kVideoTypeUnknown = 0,
    kVideoTypeI420 = 1,
    kVideoTypeIYUV = 2,
    kVideoTypeRGB24 = 3,
    kVideoTypeABGR = 4,
    kVideoTypeARGB = 5,
    kVideoTypeARGB4444 = 6,
    kVideoTypeRGB565 = 7,
    kVideoTypeARGB1555 = 8,
    kVideoTypeYUY2 = 9,
    kVideoTypeYV12 = 10,
    kVideoTypeUYVY = 11,
    kVideoTypeMJPEG = 12,
    kVideoTypeNV21 = 13,
    kVideoTypeNV12 = 14,
    kVideoTypeBGRA = 15,
};

struct VideoDescription {
    uint32_t width{};
    uint32_t height{};
    uint32_t fps{};
    VideoType video_type{};
};
