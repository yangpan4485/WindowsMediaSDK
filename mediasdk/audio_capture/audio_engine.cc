#include "audio_engine.h"

#include "device/audio_device_listener.h"
#include "device/audio_device_manager.h"
#include "device/audio_volume_listener.h"
#include <iostream>

class AudioEngine::AudioEngineImpl {
public:
    AudioEngineImpl() {
        playout_audio_devices_ = AudioDeviceManager::GetInstance().EnumPlayoutAudioDevices();
        record_audio_devices_ = AudioDeviceManager::GetInstance().EnumRecordAudioDevices();
        audio_device_listener_.SetAudioDeviceChangeCallback(
            std::bind(&AudioEngineImpl::OnAudioDeviceEvent, this, std::placeholders::_1,
                      std::placeholders::_2));
        playout_audio_volume_listener_.RegisteVolumeCllback([&](int volume, bool mute) {
            auto volume_observer = volume_observer_.lock();
            if (volume_observer) {
                volume_observer->OnAudioVolumeChange(kPlayoutAudioDevice, playout_volume_device_id_,
                                                     volume, mute);
            }
        });
        record_audio_volume_listener_.RegisteVolumeCllback([&](int volume, bool mute) {
            auto volume_observer = volume_observer_.lock();
            if (volume_observer) {
                volume_observer->OnAudioVolumeChange(kRecordAudioDevice, record_volume_device_id_,
                                                     volume, mute);
            }
        });
    }

    ~AudioEngineImpl() {}

    AudioDeviceInfo GetDefaultPlayoutAudioDevice() {
        return AudioDeviceManager::GetInstance().GetDefaultPlayoutAudioDevice();
    }

    AudioDeviceInfo GetDefaultRecordAudioDevice() {
        return AudioDeviceManager::GetInstance().GetDefaultRecordAudioDevice();
    }

    std::vector<AudioDeviceInfo> EnumPlayoutAudioDevices() {
        return playout_audio_devices_;
    }

    std::vector<AudioDeviceInfo> EnumRecordAudioDevices() {
        return record_audio_devices_;
    }

    void RegisterVolumeObserver(std::shared_ptr<IAudioVolumeObserver> volume_observer) {
        volume_observer_ = volume_observer;
    }

    void StartPlayoutDeviceVolumeListen(const std::string& device_id) {
        if (playout_volume_device_id_ != device_id && !playout_volume_device_id_.empty()) {
            playout_audio_volume_listener_.StopVolumeListen();
        }
        playout_volume_device_id_ = device_id;
        playout_audio_volume_listener_.StartVolumeListen(playout_volume_device_id_);
    }
    void StopPlayoutDeviceVolumeListen() {
        playout_audio_volume_listener_.StopVolumeListen();
    }

    void StartRecordDeviceVolumeListen(const std::string& device_id) {
        if (record_volume_device_id_ != device_id && !record_volume_device_id_.empty()) {
            record_audio_volume_listener_.StopVolumeListen();
        }
        record_volume_device_id_ = device_id;
        record_audio_volume_listener_.StartVolumeListen(record_volume_device_id_);
    }

    void StopRecordDeviceVolumeListen() {
        playout_audio_volume_listener_.StopVolumeListen();
    }

    void RegisterDeviceEventObserver(std::shared_ptr<IAudioDeviceEventObserver> event_observer) {
        event_observer_ = event_observer;
    }

    void OnAudioDeviceEvent(AudioDeviceEvent event_name, const std::string& device_id) {
        AudioDeviceInfo device_info;
        device_info.device_id = device_id;
        AudioDeviceType device_type = kUnknowAudioDeviceType;
        switch (event_name) {
        case kAudioDeviceEventAdd: {
            playout_audio_devices_ = AudioDeviceManager::GetInstance().EnumPlayoutAudioDevices();
            record_audio_devices_ = AudioDeviceManager::GetInstance().EnumRecordAudioDevices();
            for (auto iter = playout_audio_devices_.begin(); iter != playout_audio_devices_.end();
                 ++iter) {
                if (iter->device_id == device_id) {
                    device_info.device_name = iter->device_name;
                    device_type = kPlayoutAudioDevice;
                    break;
                }
            }
            for (auto iter = record_audio_devices_.begin(); iter != record_audio_devices_.end();
                 ++iter) {
                if (iter->device_id == device_id) {
                    device_info.device_name = iter->device_name;
                    device_type = kRecordAudioDevice;
                    break;
                }
            }
        } break;
        case kAudioDeviceEventRemove: {
            for (auto iter = playout_audio_devices_.begin(); iter != playout_audio_devices_.end();
                 ++iter) {
                if (iter->device_id == device_id) {
                    device_info.device_name = iter->device_name;
                    device_type = kPlayoutAudioDevice;
                    playout_audio_devices_.erase(iter);
                    break;
                }
            }
            for (auto iter = record_audio_devices_.begin(); iter != record_audio_devices_.end();
                 ++iter) {
                if (iter->device_id == device_id) {
                    device_info.device_name = iter->device_name;
                    device_type = kRecordAudioDevice;
                    record_audio_devices_.erase(iter);
                    break;
                }
            }
        } break;
        case kAudioDeviceEventDefaultDeviceChange:
            break;
        default:
            break;
        }
        auto event_observer = event_observer_.lock();
        if (event_observer) {
            event_observer->OnDeviceChange(device_type, event_name, device_info);
        }
    }

    void HandlePlayoutDevice(AudioDeviceEvent event_name, const AudioDeviceInfo& device_info) {
        switch (event_name) {
        case kAudioDeviceEventAdd: {
            playout_audio_devices_.push_back(device_info);
        } break;
        case kAudioDeviceEventRemove: {
            auto iter = playout_audio_devices_.begin();
            for (auto iter = playout_audio_devices_.begin(); iter != playout_audio_devices_.end();
                 ++iter) {
                if (iter->device_id == device_info.device_id) {
                    playout_audio_devices_.erase(iter);
                    break;
                }
            }
        } break;
        case kAudioDeviceEventDefaultDeviceChange:
            break;
        default:
            break;
        }
    }

    void HandleRecordDevice(AudioDeviceEvent event_name, const AudioDeviceInfo& device_info) {
        switch (event_name) {
        case kAudioDeviceEventAdd: {
            record_audio_devices_.push_back(device_info);
        } break;
        case kAudioDeviceEventRemove: {
            auto iter = record_audio_devices_.begin();
            for (auto iter = record_audio_devices_.begin(); iter != record_audio_devices_.end();
                 ++iter) {
                if (iter->device_id == device_info.device_id) {
                    record_audio_devices_.erase(iter);
                    break;
                }
            }
        } break;
        case kAudioDeviceEventDefaultDeviceChange:
            break;
        default:
            break;
        }
    }

private:
    std::vector<AudioDeviceInfo> playout_audio_devices_{};
    std::vector<AudioDeviceInfo> record_audio_devices_{};
    AudioDeviceListener audio_device_listener_{};
    AudioVolumeListener playout_audio_volume_listener_{};
    AudioVolumeListener record_audio_volume_listener_{};
    std::string playout_volume_device_id_{};
    std::string record_volume_device_id_{};
    std::weak_ptr<IAudioDeviceEventObserver> event_observer_{};
    std::weak_ptr<IAudioVolumeObserver> volume_observer_{};
};

AudioEngine::AudioEngine() : pimpl_(new AudioEngineImpl()) {}

AudioEngine::~AudioEngine() {}

AudioDeviceInfo AudioEngine::GetDefaultPlayoutAudioDevice() {
    return pimpl_->GetDefaultPlayoutAudioDevice();
}

AudioDeviceInfo AudioEngine::GetDefaultRecordAudioDevice() {
    return pimpl_->GetDefaultRecordAudioDevice();
}

std::vector<AudioDeviceInfo> AudioEngine::EnumPlayoutAudioDevices() {
    return pimpl_->EnumPlayoutAudioDevices();
}

std::vector<AudioDeviceInfo> AudioEngine::EnumRecordAudioDevices() {
    return pimpl_->EnumRecordAudioDevices();
}

void AudioEngine::RegisterVolumeObserver(std::shared_ptr<IAudioVolumeObserver> volume_observer) {
    pimpl_->RegisterVolumeObserver(volume_observer);
}

void AudioEngine::StartPlayoutDeviceVolumeListen(const std::string& device_id) {
    pimpl_->StartPlayoutDeviceVolumeListen(device_id);
}

void AudioEngine::StopPlayoutDeviceVolumeListen() {
    pimpl_->StopPlayoutDeviceVolumeListen();
}

void AudioEngine::StartRecordDeviceVolumeListen(const std::string& device_id) {
    pimpl_->StartRecordDeviceVolumeListen(device_id);
}

void AudioEngine::StopRecordDeviceVolumeListen() {
    pimpl_->StopRecordDeviceVolumeListen();
}

void AudioEngine::RegisterDeviceEventObserver(
    std::shared_ptr<IAudioDeviceEventObserver> event_observer) {
    pimpl_->RegisterDeviceEventObserver(event_observer);
}