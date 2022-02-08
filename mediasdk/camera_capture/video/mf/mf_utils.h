#pragma once
#include <string>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>

#include "video/video_info.h"

GUID GetMFGuidByFormat(VideoType video_type);
VideoType GetMFVideoType(GUID sub_type);