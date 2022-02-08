#include <iostream>

#include "audio_engine.h"
#include "string_utils.h"

class AudioObserver : public IAudioDeviceEventObserver, public IAudioVolumeObserver {
public:
    AudioObserver() {}
    ~AudioObserver() {}

    void OnDeviceChange(AudioDeviceType device_type, AudioDeviceEvent event,
                        const AudioDeviceInfo& device_info) override {
        std::string type_name = "";
        if (device_type == kPlayoutAudioDevice) {
            type_name = "playout audio device";
        } else if (device_type == kRecordAudioDevice) {
            type_name = "record audio device";
        }
        std::string event_name = "";
        if (event == kAudioDeviceEventAdd) {
            event_name = "device add";
        } else if (event == kAudioDeviceEventRemove) {
            event_name = "device remove";
        } else if (event == kAudioDeviceEventDefaultDeviceChange) {
            event_name = "default device change";
        }
        std::cout << type_name << ": " << event_name << std::endl;
    }

    void OnAudioVolumeChange(AudioDeviceType device_type, const std::string& device_id,
                             int current_volume, bool is_mute) override {
        std::string type_name = "";
        if (device_type == kPlayoutAudioDevice) {
            type_name = "playout audio device";
        } else if (device_type == kRecordAudioDevice) {
            type_name = "record audio device";
        }
        std::cout << type_name << " volume change volume: " << current_volume
                  << ", mute: " << is_mute << std::endl;
    }
};

class AudioTest {
public:
    AudioTest() {}
    ~AudioTest() {}

    void StartTest() {
        audio_observer_.reset(new AudioObserver());
        engine_.RegisterDeviceEventObserver(audio_observer_);
        engine_.RegisterVolumeObserver(audio_observer_);
        playout_devices_ = engine_.EnumPlayoutAudioDevices();
        std::cout << "playout audio devices" << std::endl;
        for (size_t i = 0; i < playout_devices_.size(); ++i) {
            std::cout << "device_id: " << playout_devices_[i].device_id << std::endl;
            std::cout << "device_name: " << utils::Utf8ToAscii(playout_devices_[i].device_name)
                      << std::endl;
        }

        std::cout << "================================" << std::endl;
        std::cout << "record audio devices" << std::endl;
        record_devices_ = engine_.EnumRecordAudioDevices();
        for (size_t i = 0; i < record_devices_.size(); ++i) {
            std::cout << "device_id: " << record_devices_[i].device_id << std::endl;
            std::cout << "device_name: " << utils::Utf8ToAscii(record_devices_[i].device_name)
                      << std::endl;
        }

        AudioDeviceInfo default_playout_device = engine_.GetDefaultPlayoutAudioDevice();
        AudioDeviceInfo default_record_device = engine_.GetDefaultRecordAudioDevice();

        std::cout << "deafult playout device: "
                  << utils::Utf8ToAscii(default_playout_device.device_name) << std::endl;
        std::cout << "deafult record device: "
                  << utils::Utf8ToAscii(default_record_device.device_name) << std::endl;

        engine_.StartPlayoutDeviceVolumeListen(default_playout_device.device_id);
        engine_.StartRecordDeviceVolumeListen(default_record_device.device_id);
    }

private:
    AudioEngine engine_{};
    std::shared_ptr<AudioObserver> audio_observer_{};
    std::vector<AudioDeviceInfo> playout_devices_{};
    std::vector<AudioDeviceInfo> record_devices_{};
};

int main(void) {
    AudioTest audio_test;
    audio_test.StartTest();
    getchar();
    return 0;
}