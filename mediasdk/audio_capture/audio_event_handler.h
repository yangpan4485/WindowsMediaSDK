#pragma once
#include "audio_common.h"

class IAudioDeviceEventObserver {
public:
    IAudioDeviceEventObserver() {}
    virtual ~IAudioDeviceEventObserver() {}
    virtual void OnDeviceChange(AudioDeviceType device_type, AudioDeviceEvent event,
                                const AudioDeviceInfo& device_info) = 0;
};

class IAudioVolumeObserver {
public:
    IAudioVolumeObserver() {}
    virtual ~IAudioVolumeObserver() {}
    virtual void OnAudioVolumeChange(AudioDeviceType device_type, const std::string& device_id,
                                     int current_volume, bool is_mute) = 0;
};

class IAudioFrameObserver {
public:
    IAudioFrameObserver() {}
    virtual ~IAudioFrameObserver() {}
    virtual void OnAudioFrame() = 0;
};