#include "audio_device_listener.h"

#include <iostream>

#include "string_utils.h"

class AudioNotification : public IMMNotificationClient {
public:
    AudioNotification() {}
    ~AudioNotification() {}

    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId,
                                                   DWORD dwNewState) override {
        /*std::cout << "OnDeviceStateChanged dwNewState: " << dwNewState
                  << ", pwstrDeviceId: " << utils::UnicodeToAnsi(pwstrDeviceId) << std::endl;*/
        AudioDeviceEvent device_event = kAudioDeviceEventUnknow;
        if (dwNewState == DEVICE_STATE_ACTIVE) {
            device_event = kAudioDeviceEventAdd;
        } else if (dwNewState == DEVICE_STATE_UNPLUGGED) {
            device_event = kAudioDeviceEventRemove;
        }
        if (device_callback_) {
            device_callback_(device_event, utils::UnicodeToUtf8(pwstrDeviceId));
        }
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) override {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) override {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role,
                                                     LPCWSTR pwstrDefaultDevice) override {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId,
                                                     const PROPERTYKEY key) override {
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

    void SetDeviceCallback(AudioDeviceChangeCallback callback) {
        device_callback_ = callback;
    }

private:
    float volume_{-1};
    bool mute_{false};
    long ref_count_ = 1;
    AudioDeviceChangeCallback device_callback_{};
};

AudioDeviceListener::AudioDeviceListener() : audio_notifaction_(new AudioNotification()) {
    Init();
    /*audio_notifaction_->SetDeviceCallback(
        [&](AudioDeviceEvent event_name, const std::string& device_id) {

        });*/
    enumerator_->RegisterEndpointNotificationCallback(audio_notifaction_.get());
}

AudioDeviceListener::~AudioDeviceListener() {
    Uninit();
}

void AudioDeviceListener::SetAudioDeviceChangeCallback(AudioDeviceChangeCallback callback) {
    audio_notifaction_->SetDeviceCallback(callback);
}

bool AudioDeviceListener::Init() {
    // CoInitialize(NULL);
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    HRESULT hr =
        CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
                         __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&enumerator_));
    return true;
}

bool AudioDeviceListener::Uninit() {
    if (enumerator_) {
        SAFE_RELEASE(enumerator_);
    }
    CoUninitialize();
    return true;
}