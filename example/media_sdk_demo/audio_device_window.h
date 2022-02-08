#pragma once
#include <functional>
#include <string>

#include "UIlib.h"
#include "audio_common.h"

class AudioDeviceWindow : public DuiLib::CWindowWnd,
                          public DuiLib::INotifyUI,
                          public DuiLib::IMessageFilterUI {
public:
    using VideoDeviceCallback = std::function<void(const std::string& device_id)>;

public:
    AudioDeviceWindow();
    ~AudioDeviceWindow();

    LPCTSTR GetWindowClassName() const override;
    UINT GetClassStyle() const override;
    void Notify(DuiLib::TNotifyUI& msg) override;
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void SetAudioDevies(const std::vector<AudioDeviceInfo>& playout_audio_devices,
                        const std::vector<AudioDeviceInfo>& record_audio_devices);

private:
    void InitWindow();

private:
    DuiLib::CPaintManagerUI paint_manager_{};
    std::vector<AudioDeviceInfo> playout_audio_devices_{};
    std::vector<AudioDeviceInfo> record_audio_devices_{};
    std::vector<std::string> playback_vec_;
    std::vector<std::string> record_vec_;
    std::string playback_select_id_{};
    std::string record_select_id_{};
};