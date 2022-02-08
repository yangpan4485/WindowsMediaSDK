#include "video_utils.h"

#include "libyuv.h"

VideoDescription FindVideoDescription(const std::vector<VideoDescription>& video_descs,
                                      const VideoProfile& video_profile) {
    if (video_descs.empty()) {
        return VideoDescription();
    }
    VideoDescription video_desc = video_descs[0];
    int pre_width = video_desc.width - video_profile.width;
    int pre_height = video_desc.height - video_profile.height;
    uint32_t min_size = pre_width * pre_width + pre_height * pre_height;
    for (size_t i = 1; i < video_descs.size(); ++i) {
        int width = video_descs[i].width - video_profile.width;
        int height = video_descs[i].height - video_profile.height;
        uint32_t size = width * width + height * height;
        if (size < min_size) {
            min_size = size;
            video_desc = video_descs[i];
        } else if (size == min_size) {
            if (video_descs[i].fps > video_desc.fps) {
                video_desc = video_descs[i];
            }
        }
    }
    return video_desc;
}

int ConvertVideoType(VideoType video_type) {
    switch (video_type) {
    case VideoType::kVideoTypeUnknown:
        return libyuv::FOURCC_ANY;
    case VideoType::kVideoTypeI420:
        return libyuv::FOURCC_I420;
    case VideoType::kVideoTypeIYUV: // same as VideoType::kYV12
    case VideoType::kVideoTypeYV12:
        return libyuv::FOURCC_YV12;
    case VideoType::kVideoTypeRGB24:
        return libyuv::FOURCC_24BG;
    case VideoType::kVideoTypeABGR:
        return libyuv::FOURCC_ABGR;
    case VideoType::kVideoTypeRGB565:
        return libyuv::FOURCC_RGBP;
    case VideoType::kVideoTypeYUY2:
        return libyuv::FOURCC_YUY2;
    case VideoType::kVideoTypeUYVY:
        return libyuv::FOURCC_UYVY;
    case VideoType::kVideoTypeMJPEG:
        return libyuv::FOURCC_MJPG;
    case VideoType::kVideoTypeNV21:
        return libyuv::FOURCC_NV21;
    case VideoType::kVideoTypeNV12:
        return libyuv::FOURCC_NV12;
    case VideoType::kVideoTypeARGB:
        return libyuv::FOURCC_ARGB;
    case VideoType::kVideoTypeBGRA:
        return libyuv::FOURCC_BGRA;
    case VideoType::kVideoTypeARGB4444:
        return libyuv::FOURCC_R444;
    case VideoType::kVideoTypeARGB1555:
        return libyuv::FOURCC_RGBO;
    }
    return libyuv::FOURCC_ANY;
}