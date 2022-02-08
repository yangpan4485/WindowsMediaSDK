#pragma once
#include <vector>

#include "audio_common.h"
#include "audio_event_handler.h"

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    AudioDeviceInfo GetDefaultPlayoutAudioDevice();
    AudioDeviceInfo GetDefaultRecordAudioDevice();
    std::vector<AudioDeviceInfo> EnumPlayoutAudioDevices();
    std::vector<AudioDeviceInfo> EnumRecordAudioDevices();

    void RegisterVolumeObserver(std::shared_ptr<IAudioVolumeObserver> volume_observer);
    void StartPlayoutDeviceVolumeListen(const std::string& device_id);
    void StopPlayoutDeviceVolumeListen();
    void StartRecordDeviceVolumeListen(const std::string& device_id);
    void StopRecordDeviceVolumeListen();
    void RegisterDeviceEventObserver(std::shared_ptr<IAudioDeviceEventObserver> event_observer);

private:
    class AudioEngineImpl;
    std::unique_ptr<AudioEngineImpl> pimpl_{};
};