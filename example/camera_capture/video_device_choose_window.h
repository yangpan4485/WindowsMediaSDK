#pragma once
#include <functional>
#include <string>

#include "UIlib.h"
#include "video_common.h"

class VideoDeviceWindow : public DuiLib::CWindowWnd,
                          public DuiLib::INotifyUI,
                          public DuiLib::IMessageFilterUI {
public:
    using VideoDeviceCallback = std::function<void(const std::string& device_id)>;

public:
    VideoDeviceWindow();
    ~VideoDeviceWindow();

    LPCTSTR GetWindowClassName() const override;
    UINT GetClassStyle() const override;
    void Notify(DuiLib::TNotifyUI& msg) override;
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void SetCurrentVideoDevice(const std::string& device_id);
    void SetVideoDevies(const std::vector<VideoDeviceInfo>& video_devices);
    void SetVideoDeviceCallback(VideoDeviceCallback callback);

private:
    void InitWindow();

private:
    DuiLib::CPaintManagerUI paint_manager_{};
    std::vector<VideoDeviceInfo> video_devices_;

    std::vector<std::string> option_vec_{};
    std::string select_id_{};
    VideoDeviceCallback callback_{};
};