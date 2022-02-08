#pragma once
#include <vector>

#include <wmcodecdsp.h> // CLSID_CWMAudioAEC
// (must be before audioclient.h)
#include <audioclient.h> // WASAPI
#include <audiopolicy.h>
#include <avrt.h> // Avrt
#include <endpointvolume.h>
#include <mediaobj.h>    // IMediaObject
#include <mmdeviceapi.h> // MMDevice

#include "../audio_common.h"

class AudioDeviceManager {
public:
    static AudioDeviceManager& GetInstance();

    AudioDeviceInfo GetDefaultPlayoutAudioDevice();
    AudioDeviceInfo GetDefaultRecordAudioDevice();
    std::vector<AudioDeviceInfo> EnumPlayoutAudioDevices();
    std::vector<AudioDeviceInfo> EnumRecordAudioDevices();

private:
    AudioDeviceManager();
    ~AudioDeviceManager();
    AudioDeviceManager(const AudioDeviceManager&) = delete;
    AudioDeviceManager operator=(const AudioDeviceManager&) = delete;

    std::vector<AudioDeviceInfo> GetAudioDevice(EDataFlow flow);
    AudioDeviceInfo GetDefaultAudioDevice(EDataFlow flow);
    AudioDeviceInfo GetDevice(IMMDevice* pDevice);
    bool Init();
    bool Uninit();

private:
    IMMDeviceEnumerator* enumerator_{};
    IMMDeviceCollection* render_collection_{};
    IMMDeviceCollection* capture_collection_{};
    bool init_{};
};