#include "main_window.h"

#include <fstream>
#include <iostream>
#include <string>

#include <VersionHelpers.h>
#include <shellscalingapi.h>

#include "my_window.h"
#include "video_render_factory.h"

#pragma comment(lib, "Shcore.lib")

MainWindow::MainWindow() {
    video_capture_engine_.reset(new VideoCaptureEngine());
    video_frame_observer_.reset(new VideoFrameObserver());
    video_frame_observer_->SetObserver(this);
    audio_engine_.reset(new AudioEngine());
    screen_capture_engine_.reset(new ScreenCaptureEngine());
    screen_capture_handler_.reset(new ScreenCaptureHandler());
    screen_capture_handler_->SetObserver(this);
    screen_capture_engine_->RegisterCaptureHandler(screen_capture_handler_);
    video_encoder_ = VideoEnocderFcatory::Instance().CreateEncoder(kEncodeTypeQSV);
	encode_fout_.open("../../../encode.h264", std::ios::binary | std::ios::out);
	video_encoder_->SetOutputSize(1280, 720);
    video_encoder_->RegisterEncodeCalback([&](uint8_t* data, uint32_t len) {
        encode_fout_.write((char*)data, len);
		if (socket_client_window_ && socket_client_window_->IsConnect()) {
			socket_client_window_->SendFrame(data, len);
		}
    });
}

MainWindow::~MainWindow() {
    running_ = false;
    cv_.notify_all();
    if (render_thread_.joinable()) {
        render_thread_.join();
    }
    encode_running_ = false;
    encode_cv_.notify_all();
    if (encode_work_thread_.joinable()) {
        encode_work_thread_.join();
    }
    encode_fout_.close();
}

void MainWindow::Init() {
    if (IsWindows8OrGreater()) {
        SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    }
    hinstance_ = GetModuleHandle(0);
    DuiLib::CPaintManagerUI::SetInstance(hinstance_);
    std::string path = "..\\..\\..\\example\\media_sdk_demo\\xml";
    DuiLib::CPaintManagerUI::SetResourcePath(DuiLib::CPaintManagerUI::GetInstancePath() +
                                             path.c_str());
}

void MainWindow::CreateDuiWindow() {
    Create(NULL, _T("MainWindow"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
}

void MainWindow::Show() {
    ShowModal();
}

LPCTSTR MainWindow::GetWindowClassName() const {
    return _T("DUIMainFrame");
}

void MainWindow::Notify(DuiLib::TNotifyUI& msg) {
    if (msg.sType == _T("click")) {
        OnClick(msg);
    }
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LRESULT lRes = 0;
    switch (uMsg) {
    case WM_CREATE:
        lRes = OnCreate(uMsg, wParam, lParam);
        break;
    case WM_CLOSE:
        lRes = OnClose(uMsg, wParam, lParam);
        break;
    case WM_TIMER:
        lRes = OnTimer(uMsg, wParam, lParam);
        break;
    case WM_SIZE:
        break;
    default:
        break;
    }
    if (paint_manager_.MessageHandler(uMsg, wParam, lParam, lRes)) {
        return lRes;
    }
    return __super::HandleMessage(uMsg, wParam, lParam);
}

LRESULT MainWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    paint_manager_.Init(m_hWnd);
    DuiLib::CDialogBuilder builder;
    DuiLib::CControlUI* pRoot =
        builder.Create(_T("main_window.xml"), (UINT)0, this, &paint_manager_);
    paint_manager_.AttachDialog(pRoot);
    paint_manager_.AddNotifier(this);
    CenterWindow();
    return 0;
}

LRESULT MainWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    CloseVideo();
    return 0;
}

LRESULT MainWindow::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return 0;
}

void MainWindow::OnClick(DuiLib::TNotifyUI& msg) {
    auto name = msg.pSender->GetName();
    std::cout << "name: " << name << std::endl;
    if (name == "btnOpen") {
        DuiLib::CButtonUI* open_video_ =
            (DuiLib::CButtonUI*)(paint_manager_.FindControl(_T("btnOpen")));
        if (is_open_video_) {
            CloseVideo();
            open_video_->SetText("开启视频");
            is_open_video_ = false;
        } else {
            OpenVideo();
            open_video_->SetText("关闭视频");
            is_open_video_ = true;
        }
    } else if (name == "btnVideoDevice") {
        CreateVideoDeviceWindow();
    } else if (name == "btnAudioDevice") {
        CreateAudioDeviceWindow();
    } else if (name == "btnScreenShare") {
        CreateScreenShareWindow();
    } else if (name == "btnScreenStop") {
        screen_capture_engine_->StopCapture();
    }
	else if (name == "btnSocket") {
		CreateSocketClientWindow();
	}
	else if (name == "btnQuit") {
        Close();
    }
}

DuiLib::CControlUI* MainWindow::CreateControl(LPCTSTR pstrClass) {
    if (_tcscmp(pstrClass, _T("CWndUI")) == 0) {
        CWndUI* wndui = new CWndUI();
        HWND wnd = CreateWindow(_T("STATIC"), _T(""),
                                WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, 0,
                                0, paint_manager_.GetPaintWindow(), (HMENU)0, NULL, NULL);
        EnableWindow(wnd, FALSE);
        wndui->Attach(wnd);
        wndui->SetEnabled(false);
        return wndui;
    }
    return NULL;
}

bool MainWindow::OpenVideo() {
    if (is_capture_video_) {
        return true;
    }
    if (video_devices_.empty()) {
        video_devices_ = video_capture_engine_->EnumVideoDevices();
    }
    if (video_devices_.empty()) {
        return false;
    }
    if (video_device_id_.empty() && !video_devices_.empty()) {
        video_device_id_ = video_devices_[0].device_id;
    }
    std::cout << "video_device_id_: " << video_device_id_ << std::endl;
    video_capture_engine_->RegisteVideoFrameObserver(video_frame_observer_);
    video_capture_engine_->RegisteVideoDeviceEventHandler(video_frame_observer_);
    video_capture_engine_->StartCapture(video_device_id_);
    is_capture_video_ = true;
    return true;
}

bool MainWindow::CloseVideo() {
    if (!is_capture_video_) {
        return true;
    }
    video_capture_engine_->StopCapture();
    is_capture_video_ = false;
    return true;
}

void MainWindow::OnVideoFrame(std::shared_ptr<VideoFrame> video_frame) {
    {
        std::unique_lock<std::mutex> lock(mtx_);
        frame_queue_.push(video_frame);
        cv_.notify_all();
    }
    if (video_render_) {
        return;
    }
    video_render_ = VideoRenderFactory::CreateInstance()->CreateVideoRender(kRenderTypeOpenGL);
    auto wnd = (CWndUI*)(paint_manager_.FindControl("renderWindow"));
    wnd->SetEnabled(false);
    wnd->SetVisible(true);
    wnd->SetPos({0, 0, 1200, 600});
    ::ShowWindow(wnd->GetHwnd(), true);
    HWND hwnd = wnd->GetHwnd();
    video_render_->SetWindow(hwnd);
    running_ = true;
    if (render_thread_.joinable()) {
        render_thread_.join();
    }
    render_thread_ = std::thread([&]() {
        while (running_) {
            std::shared_ptr<VideoFrame> frame;
            {
                std::unique_lock<std::mutex> lock(mtx_);
                if (frame_queue_.empty()) {
                    cv_.wait(lock);
                }
                if (!running_) {
                    return;
                }
                frame = frame_queue_.front();
                frame_queue_.pop();
            }
            uint32_t width = frame->GetWidth();
            uint32_t height = frame->GetHeight();
            uint8_t* y_data = frame->GetData();
            uint8_t* u_data = y_data + width * height;
            uint8_t* v_data = y_data + width * height * 5 / 4;
            video_render_->RendFrameI420(y_data, width, u_data, width / 2, v_data, width / 2, width,
                                         height);
        }
    });
}

void MainWindow::CreateVideoDeviceWindow() {
    if (video_devices_.empty()) {
        video_devices_ = video_capture_engine_->EnumVideoDevices();
    }
    if (video_device_window_ && IsWindow(video_device_hwnd_)) {
        return;
    }
    int count = video_devices_.size();
    int width = 600;
    int height = count * 50 + 70;
    video_device_window_.reset(new VideoDeviceWindow());
    video_device_window_->SetVideoDeviceCallback([&](const std::string& device_id) {
        video_device_id_ = device_id;
        if (is_capture_video_) {
            CloseVideo();
            OpenVideo();
        }
    });
    std::string id = video_capture_engine_->GetCurrentDevice();
    if (!id.empty()) {
        video_device_id_ = id;
    }
    video_device_window_->SetCurrentVideoDevice(video_device_id_);
    video_device_window_->SetVideoDevies(video_devices_);
    video_device_hwnd_ = video_device_window_->Create(m_hWnd, _T("VideoDeviceWindow"),
                                                      UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
    video_device_window_->ResizeClient(width, height);
    video_device_window_->CenterWindow();
    this->ShowWindow(true);
}

void MainWindow::CreateAudioDeviceWindow() {
    if (playout_audio_devices_.empty()) {
        playout_audio_devices_ = audio_engine_->EnumPlayoutAudioDevices();
    }
    if (record_audio_devices_.empty()) {
        record_audio_devices_ = audio_engine_->EnumRecordAudioDevices();
    }
    if (audio_device_window_ && IsWindow(audio_device_hwnd_)) {
        return;
    }
    int playback_count = playout_audio_devices_.size();
    int record_count = record_audio_devices_.size();

    int width = 600;
    int height = (playback_count + record_count) * 50 + 2 * 60;
    audio_device_window_.reset(new AudioDeviceWindow());
    audio_device_window_->SetAudioDevies(playout_audio_devices_, record_audio_devices_);
    // audio_device_window_->SetCurrentAudioDevice(playout_audio_device_id_,
    // record_audio_device_id_);
    audio_device_hwnd_ = audio_device_window_->Create(m_hWnd, _T("AudioDeviceWindow"),
                                                      UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
    audio_device_window_->ResizeClient(width, height);
    audio_device_window_->CenterWindow();
    this->ShowWindow(true);
}

void MainWindow::CreateScreenShareWindow() {
    if (screen_share_window_ && IsWindow(screen_share_hwnd_)) {
        return;
    }
    screen_share_window_.reset(new ScreenShareWindow());
    screen_share_window_->SetScreenShareEngine(screen_capture_engine_);
    screen_share_window_->SetScreenWindowCallback(
        [&](const ScreenInfo& screen_info, const WindowInfo& window_info, bool is_screen) {
            CaptureConfig config;
            config.frame_rate = 30;
            config.enable_border = true;
            if (is_screen) {
                screen_capture_engine_->StartCaptureDisplay(screen_info, config, {this->GetHWND()});
            } else {
                screen_capture_engine_->StartCaptureWindow(window_info, config);
            }
        });
    screen_share_hwnd_ = screen_share_window_->Create(m_hWnd, _T("ScreenShareWindow"),
                                                      UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
    screen_share_window_->CenterWindow();
    this->ShowWindow(true);
}

void MainWindow::CreateSocketClientWindow() {
	if (socket_client_window_ && IsWindow(socket_client_hwnd_)) {
		return;
	}
	socket_client_window_.reset(new SocketClientWindow());
	socket_client_hwnd_ = socket_client_window_->Create(m_hWnd, _T("SocketClientWindow"),
		UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
	socket_client_window_->CenterWindow();
	this->ShowWindow(true);
}

void MainWindow::OnScreenFrame(std::shared_ptr<VideoFrame> video_frame) {
    if (video_frame->GetFrameType() != kFrameTypeARGB) {
        return;
    }
    {
        std::unique_lock<std::mutex> lock(encode_mtx_);
        screen_frame_queue_.push(video_frame);
        encode_cv_.notify_all();
    }
    if (encode_running_) {
        return;
    }
    encode_running_ = true;
    if (encode_work_thread_.joinable()) {
        encode_work_thread_.join();
    }
    encode_work_thread_ = std::thread([&]() {
        while (encode_running_) {
            std::shared_ptr<VideoFrame> frame;
            {
                std::unique_lock<std::mutex> lock(encode_mtx_);
                if (screen_frame_queue_.empty()) {
                    encode_cv_.wait(lock);
                }
                if (!encode_running_) {
                    return;
                }
                frame = screen_frame_queue_.front();
                screen_frame_queue_.pop();
            }
            
            video_encoder_->EncodeFrame(frame, false);
        }
    });
}

void MainWindow::OnScreenEvent(ScreenEvent screen_event) {
    if (screen_event == kScreenEventRemove) {
        screen_capture_engine_->StopCapture();
    }
}

void MainWindow::OnVideoDeviceEvent(VideoDeviceEvent device_event, const std::string& device_id) {
    if (device_event == kVideoDeviceAdd) {
        video_devices_ = video_capture_engine_->EnumVideoDevices();
    } else {
        auto iter = video_devices_.begin();
        while (iter != video_devices_.end()) {
            if (iter->device_id == device_id) {
                video_devices_.erase(iter);
                break;
            }
            ++iter;
        }
    }
}