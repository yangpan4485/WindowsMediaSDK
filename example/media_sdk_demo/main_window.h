#pragma once
#include <condition_variable>
#include <fstream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "UIlib.h"
#include "audio_common.h"
#include "audio_device_window.h"
#include "audio_engine.h"
#include "screen_capture_engine.h"
#include "screen_common.h"
#include "screen_handler.h"
#include "screen_share_window.h"
#include "video_capture_engine.h"
#include "video_common.h"
#include "video_device_window.h"
#include "video_encoder_factory.h"
#include "video_handler.h"
#include "video_render_factory.h"
#include "socket_client_window.h"

class MainWindow : public DuiLib::CWindowWnd,
                   public DuiLib::INotifyUI,
                   DuiLib::IDialogBuilderCallback {
public:
    MainWindow();
    ~MainWindow();

    void Init();
    void CreateDuiWindow();
    void Show();

    void OnVideoFrame(std::shared_ptr<VideoFrame> video_frame);
    void OnScreenFrame(std::shared_ptr<VideoFrame> video_frame);
    void OnScreenEvent(ScreenEvent screen_event);
    void OnVideoDeviceEvent(VideoDeviceEvent device_event, const std::string& device_id);

private:
    LPCTSTR GetWindowClassName() const override;
    void Notify(DuiLib::TNotifyUI& msg) override;
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnClick(DuiLib::TNotifyUI& msg);
    DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass) override;

    bool OpenVideo();
    bool CloseVideo();

    void CreateVideoDeviceWindow();
    void CreateAudioDeviceWindow();
    void CreateScreenShareWindow();
	void CreateSocketClientWindow();

private:
    DuiLib::CPaintManagerUI paint_manager_{};
    HINSTANCE hinstance_{};

    bool is_open_video_{};
    bool is_capture_video_{};
    std::shared_ptr<VideoCaptureEngine> video_capture_engine_{};
    std::shared_ptr<VideoFrameObserver> video_frame_observer_{};
    std::vector<VideoDeviceInfo> video_devices_{};
    std::string video_device_id_{};

    std::mutex mtx_;
    std::queue<std::shared_ptr<VideoFrame>> frame_queue_{};
    std::thread render_thread_{};
    bool running_{};
    std::condition_variable cv_{};
    std::shared_ptr<VideoRender> video_render_{};

    std::shared_ptr<VideoDeviceWindow> video_device_window_{};
    HWND video_device_hwnd_{};

    std::shared_ptr<AudioEngine> audio_engine_{};
    std::shared_ptr<AudioDeviceWindow> audio_device_window_{};
    HWND audio_device_hwnd_{};
    std::vector<AudioDeviceInfo> playout_audio_devices_{};
    std::vector<AudioDeviceInfo> record_audio_devices_{};

    std::shared_ptr<ScreenCaptureEngine> screen_capture_engine_{};
    std::shared_ptr<ScreenShareWindow> screen_share_window_{};
    std::shared_ptr<ScreenCaptureHandler> screen_capture_handler_{};
    HWND screen_share_hwnd_{};
    std::shared_ptr<VideoEncoder> video_encoder_{};
    std::ofstream encode_fout_{};

    std::thread encode_work_thread_{};
    bool encode_running_{};
    std::condition_variable encode_cv_{};
    std::mutex encode_mtx_;
    std::queue<std::shared_ptr<VideoFrame>> screen_frame_queue_{};
	std::shared_ptr<SocketClientWindow> socket_client_window_{};
	HWND socket_client_hwnd_{};
};
