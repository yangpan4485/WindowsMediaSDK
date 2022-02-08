#include "audio_volume_listener.h"

#include <iostream>

#include "string_utils.h"

class EndpointVolumeCallback : public IAudioEndpointVolumeCallback {
public:
    EndpointVolumeCallback() {}
    ~EndpointVolumeCallback() {}

    STDMETHODIMP OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA NotificationData) override {
        int volume = NotificationData->fMasterVolume * 100;
        bool mute = NotificationData->bMuted;
        if (volume_ != volume || mute != mute_) {
            volume_ = volume;
            mute_ = mute;
            if (volume_observer_) {
                volume_observer_->VolumeChange(volume_, mute);
            }
        }
        return S_OK;
    }

    STDMETHODIMP_(ULONG) AddRef() override {
        return InterlockedIncrement(&ref_count_);
    }

    STDMETHODIMP_(ULONG) Release() override {
        return InterlockedDecrement(&ref_count_);
    }

    STDMETHODIMP QueryInterface(REFIID IID, void** ReturnValue) override {
        if (IID == IID_IUnknown || IID == __uuidof(IAudioEndpointVolumeCallback) ||
            IID == __uuidof(IMMNotificationClient)) {
            *ReturnValue = this;
            AddRef();
            return S_OK;
        }
        *ReturnValue = NULL;
        return E_NOINTERFACE;
    }

    void RegisteVolmeObserver(AudioVolumeListener* observer) {
        volume_observer_ = observer;
    }

private:
    float volume_{-1};
    bool mute_{false};
    long ref_count_ = 1;
    AudioVolumeListener* volume_observer_{};
};

class SessionVolumeCallback : public IAudioSessionEvents {
public:
    SessionVolumeCallback() {}
    ~SessionVolumeCallback() {}

    STDMETHODIMP_(ULONG) AddRef() override {
        return InterlockedIncrement(&ref_count_);
    }

    STDMETHODIMP_(ULONG) Release() override {
        return InterlockedDecrement(&ref_count_);
    }

    STDMETHODIMP QueryInterface(REFIID IID, void** ReturnValue) override {
        if (IID == IID_IUnknown || IID == __uuidof(IAudioEndpointVolumeCallback) ||
            IID == __uuidof(IMMNotificationClient)) {
            *ReturnValue = this;
            AddRef();
            return S_OK;
        }
        *ReturnValue = NULL;
        return E_NOINTERFACE;
    }

    HRESULT STDMETHODCALLTYPE OnDisplayNameChanged(LPCWSTR NewDisplayName,
                                                   LPCGUID EventContext) override {
        return 0;
    }
    HRESULT STDMETHODCALLTYPE OnIconPathChanged(LPCWSTR NewIconPath,
                                                LPCGUID EventContext) override {
        return 0;
    }
    HRESULT STDMETHODCALLTYPE OnChannelVolumeChanged(
        DWORD ChannelCount, _In_reads_(ChannelCount) float NewChannelVolumeArray[],
        DWORD ChangedChannel, LPCGUID EventContext) override {
        return 0;
    }
    HRESULT STDMETHODCALLTYPE OnGroupingParamChanged(LPCGUID NewGroupingParam,
                                                     LPCGUID EventContext) override {
        return 0;
    }
    HRESULT STDMETHODCALLTYPE OnStateChanged(AudioSessionState NewState) override {
        return 0;
    }
    HRESULT STDMETHODCALLTYPE
    OnSessionDisconnected(AudioSessionDisconnectReason DisconnectReason) override {
        static const char* reasons[] = {
            "DisconnectReasonDeviceRemoval",       "DisconnectReasonServerShutdown",
            "DisconnectReasonFormatChanged",       "DisconnectReasonSessionLogoff",
            "DisconnectReasonSessionDisconnected", "DisconnectReasonExclusiveModeOverride"};

        if (DisconnectReason >= DisconnectReasonDeviceRemoval &&
            DisconnectReason <= DisconnectReasonExclusiveModeOverride) {
            std::cout << "DisconnectReason = " << reasons[DisconnectReason] << std::endl;
        }
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnSimpleVolumeChanged(float NewVolume, BOOL NewMute,
                                                    LPCGUID EventContext) override {
        return S_OK;
    }

private:
    float volume_{-1};
    bool mute_{false};
    long ref_count_ = 1;
};

AudioVolumeListener::AudioVolumeListener()
    : audio_endpoint_notifaction_(new EndpointVolumeCallback()),
      audio_session_notifaction_(new SessionVolumeCallback()) {
    Init();
    audio_endpoint_notifaction_->RegisteVolmeObserver(this);
}

AudioVolumeListener::~AudioVolumeListener() {
    Uninit();
}

void AudioVolumeListener::StartVolumeListen(const std::string& device_id) {
    HRESULT hr = S_OK;
    if (running_) {
        if (device_id_ != device_id) {
            StopVolumeListen();
        } else {
            return;
        }
    }
    running_ = true;
    if (device_id_ != device_id) {
        device_id_ = device_id;
        Destroy();
        hr = enumerator_->GetDevice(utils::Utf8ToUnicode(device_id).c_str(), &audio_device_);
        hr = audio_device_->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL,
                                     reinterpret_cast<void**>(&audio_endpoint_volume_));
        IAudioSessionManager* audio_session_manager_{};
        hr = audio_device_->Activate(__uuidof(IAudioSessionManager), CLSCTX_INPROC_SERVER, NULL,
                                     reinterpret_cast<void**>(&audio_session_manager_));
        hr = audio_session_manager_->GetAudioSessionControl(NULL, 0, &audio_session_control_);
        hr = audio_session_manager_->GetSimpleAudioVolume(NULL, FALSE, &simple_audio_volume_);
        SAFE_RELEASE(audio_session_manager_);
    }
    hr = audio_endpoint_volume_->RegisterControlChangeNotify(audio_endpoint_notifaction_.get());
    hr = audio_session_control_->RegisterAudioSessionNotification(audio_session_notifaction_.get());
}

void AudioVolumeListener::StopVolumeListen() {
    if (!running_) {
        return;
    }
    running_ = false;
    if (audio_endpoint_volume_) {
        audio_endpoint_volume_->UnregisterControlChangeNotify(audio_endpoint_notifaction_.get());
    }
    if (audio_session_control_) {
        audio_session_control_->UnregisterAudioSessionNotification(
            audio_session_notifaction_.get());
    }
}

void AudioVolumeListener::RegisteVolumeCllback(VolumeCallback callback) {
    volume_callback_ = callback;
}

void AudioVolumeListener::VolumeChange(int volume, bool mute) {
    if (volume_callback_) {
        volume_callback_(volume, mute);
    }
}

bool AudioVolumeListener::Init() {
    // CoInitialize(NULL);
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    HRESULT hr =
        CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
                         __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&enumerator_));
    if (FAILED(hr)) {
        return false;
    }
    return true;
}

bool AudioVolumeListener::Uninit() {
    Destroy();
    if (enumerator_) {
        SAFE_RELEASE(enumerator_);
    }
    return true;
}

void AudioVolumeListener::Destroy() {
    if (simple_audio_volume_) {
        SAFE_RELEASE(simple_audio_volume_);
    }
    if (audio_session_control_) {
        SAFE_RELEASE(audio_session_control_);
    }
    if (audio_endpoint_volume_) {
        SAFE_RELEASE(audio_endpoint_volume_);
    }
    if (audio_device_) {
        SAFE_RELEASE(audio_device_);
    }
}