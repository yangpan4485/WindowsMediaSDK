#pragma once

#include <initguid.h> 
#include <vector>
#include <dshow.h>
#include <cguid.h>
#include <dvdmedia.h>

#include "video/video_info.h"

DEFINE_GUID(MEDIASUBTYPE_I420, 0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
DEFINE_GUID(MEDIASUBTYPE_HDYC, 0x43594448, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
DEFINE_GUID(CLSID_SINKFILTER, 0x88cdbbdc, 0xa73b, 0x4afa, 0xac, 0xbf, 0x15, 0xd5, 0xe2, 0xce, 0x12, 0xc3);

IPin* GetOutputPin(IBaseFilter* filter, REFGUID category);
IPin* GetInputPin(IBaseFilter* filter);
LONGLONG GetMaxOfFrameArray(LONGLONG* max_fps_array, long size);
void ResetMediaType(AM_MEDIA_TYPE* media_type);
void FreeMediaType(AM_MEDIA_TYPE* media_type);
VideoType GetVideoType(GUID type);
BOOL PinMatchesCategory(IPin* pin, REFGUID category);
void GetSampleProperties(IMediaSample* sample, AM_SAMPLE2_PROPERTIES* props);
bool TranslateMediaTypeToVideoCaptureCapability(const AM_MEDIA_TYPE* media_type, VideoDescription* capability);
HRESULT CopyMediaType(AM_MEDIA_TYPE* target, const AM_MEDIA_TYPE* source);
bool IsMediaTypeFullySpecified(const AM_MEDIA_TYPE& type);
bool IsMediaTypePartialMatch(const AM_MEDIA_TYPE& a, const AM_MEDIA_TYPE& b);
void SetMediaInfoFromVideoType(VideoType video_type, BITMAPINFOHEADER* bitmap_header, AM_MEDIA_TYPE* media_type);
BYTE* AllocMediaTypeFormatBuffer(AM_MEDIA_TYPE* media_type, ULONG length);
