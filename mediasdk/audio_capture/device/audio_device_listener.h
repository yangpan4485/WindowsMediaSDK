#pragma once
#include <functional>
#include <memory>
#include <string>

#include <Audioclient.h>
#include <Endpointvolume.h>
#include <Mmdeviceapi.h>
#include <Windows.h>
#include <functiondiscoverykeys_devpkey.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <wrl/client.h>

#include "../audio_common.h"

class AudioNotification;
using AudioDeviceChangeCallback =
    std::function<void(AudioDeviceEvent event_name, const std::string& device_info)>;
class AudioDeviceListener {
public:
    AudioDeviceListener();
    ~AudioDeviceListener();

    void SetAudioDeviceChangeCallback(AudioDeviceChangeCallback callback);

private:
    bool Init();
    bool Uninit();

private:
    AudioDeviceChangeCallback device_change_callback_{};
    IMMDeviceEnumerator* enumerator_{};
    IMMDevice* audio_device_{};
    IAudioEndpointVolume* audio_endpoint_volume_{};
    std::shared_ptr<AudioNotification> audio_notifaction_{};
};