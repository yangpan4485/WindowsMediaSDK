#pragma once
#include <vector>

#include "video/video_info.h"
#include "video_common.h"

VideoDescription FindVideoDescription(const std::vector<VideoDescription>& video_descs,
                                      const VideoProfile& video_profile);

int ConvertVideoType(VideoType video_type);