#include "mf_utils.h"

#include "video_device_mf.h"

GUID GetMFGuidByFormat(VideoType video_type) {
    switch (video_type) {
    case kVideoTypeNV12:
        return MFVideoFormat_NV12;
    case kVideoTypeMJPEG:
        return MFVideoFormat_MJPG;
    default:
        break;
    }
    return MFVideoFormat_Base;
}

VideoType GetMFVideoType(GUID sub_type) {
    VideoType video_type = kVideoTypeUnknown;
    if (MFVideoFormat_NV12 == sub_type) {
        video_type = kVideoTypeNV12;
    } else if (MFVideoFormat_MJPG == sub_type) {
        video_type = kVideoTypeMJPEG;
    }
    return video_type;
}