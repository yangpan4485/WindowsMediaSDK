#pragma once
#include <string>

#define SAFE_RELEASE(p) \
  do {                  \
    if ((p)) {          \
      (p)->Release();   \
      (p) = NULL;       \
    }                   \
  } while (0)

#define CHECK_HRESULT(hr) \
  if(FAILED(hr)) {        \
    return false;         \
  } 

enum AudioDeviceEvent {
    kAudioDeviceEventUnknow = 0,
    kAudioDeviceEventAdd = 1,
    kAudioDeviceEventRemove = 2,
    kAudioDeviceEventDefaultDeviceChange = 3,
};

enum AudioDeviceType {
    kUnknowAudioDeviceType = 0,
    kPlayoutAudioDevice = 1,
    kRecordAudioDevice = 2,
};

struct AudioDeviceInfo {
    std::string device_name;
    std::string device_id;
};
