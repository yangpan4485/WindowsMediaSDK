#pragma once
#include <memory>
#include <thread>
#include <vector>
#include <fstream>
#include <mutex>
#include <queue>
#include <condition_variable>

#include "UIlib.h"
#include "video_capture_engine.h"
#include "video_common.h"
#include "video_device_choose_window.h"
#include "video_device_event_handler.h"
#include "video_render.h"

class MainWindow : public DuiLib::CWindowWnd,
                   public DuiLib::INotifyUI,
                   DuiLib::IDialogBuilderCallback {
public:
    MainWindow();
    ~MainWindow();

    void Init();
    void CreateDuiWindow();
    void Show();

    void OnFrame(std::shared_ptr<VideoFrame> video_frame);

private:
    LPCTSTR GetWindowClassName() const override;
    void Notify(DuiLib::TNotifyUI& msg) override;
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnClick(DuiLib::TNotifyUI& msg);
    DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass) override;

    void CreateVideoDeviceChooseWindow();
    void OpenVideo();
    void CloseVideo();

private:
    DuiLib::CPaintManagerUI paint_manager_{};
    HINSTANCE hinstance_{};
    std::shared_ptr<VideoRender> video_render_{};
    std::shared_ptr<VideoFrameObserver> video_frame_observer_{};
    std::shared_ptr<VideoDeviceWindow> video_device_window_{};
    std::shared_ptr<VideoCaptureEngine> video_capture_engine_{};

    bool is_capture_{};
    std::vector<VideoDeviceInfo> video_devices_{};
    std::string current_device_id_{};
    HWND video_device_hwnd_{};
    
    std::mutex mtx_;
    std::queue<std::shared_ptr<VideoFrame>> frame_queue_{};
    std::thread render_thread_{};
    bool running_{};
    std::condition_variable cv_{};
};
