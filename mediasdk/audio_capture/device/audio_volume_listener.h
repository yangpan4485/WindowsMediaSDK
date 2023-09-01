#pragma once
#include <Audioclient.h>
#include <Endpointvolume.h>
#include <Mmdeviceapi.h>
#include <Windows.h>
#include <audiopolicy.h>
#include <functiondiscoverykeys_devpkey.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <wrl/client.h>
#include <functional>
#include <memory>
#include "audio_common.h"

class EndpointVolumeCallback;
class SessionVolumeCallback;

class AudioVolumeListener {
public:
    using VolumeCallback = std::function<void(int volume, bool mute)>;
public:
    AudioVolumeListener();
    ~AudioVolumeListener();

    void StartVolumeListen(const std::string& device_id);
    void StopVolumeListen();
    void RegisteVolumeCllback(VolumeCallback callback);

    bool Init();
    bool Uninit();
    void Destroy();
    void VolumeChange(int volume, bool mute);

private:
    IMMDeviceEnumerator* enumerator_{};
    IMMDevice* audio_device_{};
    IAudioEndpointVolume* audio_endpoint_volume_{};
    IAudioSessionControl* audio_session_control_{};
    ISimpleAudioVolume* simple_audio_volume_{};
    std::shared_ptr<EndpointVolumeCallback> audio_endpoint_notifaction_{};
    std::shared_ptr<SessionVolumeCallback> audio_session_notifaction_{};
    std::string device_id_{};
    bool running_{};
    VolumeCallback volume_callback_{};
};