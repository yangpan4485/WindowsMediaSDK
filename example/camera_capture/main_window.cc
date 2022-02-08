#include "main_window.h"

#include <fstream>
#include <iostream>
#include <string>

#include <VersionHelpers.h>
#include <shellscalingapi.h>

#include "my_window.h"
#include "video_render_factory.h"

#pragma comment(lib, "Shcore.lib")

MainWindow::MainWindow() : video_capture_engine_(new VideoCaptureEngine()) {
}

MainWindow::~MainWindow() {
    running_ = false;
    cv_.notify_all();
    if (render_thread_.joinable()) {
        render_thread_.join();
    }
}

void MainWindow::Init() {
    if (IsWindows8OrGreater()) {
        SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    }
    hinstance_ = GetModuleHandle(0);
    DuiLib::CPaintManagerUI::SetInstance(hinstance_);
    std::string path = "..\\..\\..\\example\\camera_capture\\xml";
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
    if (is_capture_) {
        CloseVideo();
    }
    return 0;
}

LRESULT MainWindow::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return 0;
}

void MainWindow::OnClick(DuiLib::TNotifyUI& msg) {
    auto name = msg.pSender->GetName();
    if (name == "btnQuit") {
        std::cout << "btn quit" << std::endl;
        Close();
        return;
    } else if (name == "btnOpen") {
        OpenVideo();
    } else if (name == "btnClose") {
        CloseVideo();
    } else if (name == "btnChoose") {
        CreateVideoDeviceChooseWindow();
    }
}

void MainWindow::OpenVideo() {
    if (is_capture_) {
        return;
    }
    if (video_devices_.empty()) {
        video_devices_ = video_capture_engine_->EnumVideoDevices();
    }
    if (video_devices_.empty()) {
        return;
    }
    if (current_device_id_.empty()) {
        current_device_id_ = video_devices_[0].device_id;
    }
    if (!video_frame_observer_) {
        video_frame_observer_.reset(new VideoFrameObserver());
        video_frame_observer_->SetObserver(this);
    }
    video_capture_engine_->RegisteVideoFrameObserver(video_frame_observer_);
    VideoProfile video_profile;
    video_profile.width = 1280;
    video_profile.height = 720;
    video_profile.fps = 30;
    video_capture_engine_->SetVideoProfile(video_profile);
    is_capture_ = true;
    video_capture_engine_->StartCapture(current_device_id_);
}

void MainWindow::CloseVideo() {
    if (is_capture_) {
        video_capture_engine_->StopCapture();
        is_capture_ = false;
    }
}

void MainWindow::CreateVideoDeviceChooseWindow() {
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
    video_device_window_->SetVideoDeviceCallback(
        [&](const std::string& device_id) { current_device_id_ = device_id; });
    video_device_window_->SetCurrentVideoDevice(current_device_id_);
    video_device_window_->SetVideoDevies(video_devices_);
    video_device_hwnd_ = video_device_window_->Create(m_hWnd, _T("VideoDeviceWindow"),
                                                      UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
    video_device_window_->ResizeClient(width, height);
    video_device_window_->CenterWindow();
    this->ShowWindow(true);
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

void MainWindow::OnFrame(std::shared_ptr<VideoFrame> video_frame) {
    {
        std::unique_lock<std::mutex> lock(mtx_);
        if (frame_queue_.size() > 2) {
            frame_queue_.pop();
        }
        frame_queue_.push(video_frame);
    }
    cv_.notify_all();
    if (video_render_) {
        return;
    }
    video_render_ = VideoRenderFactory::CreateInstance()->CreateVideoRender(kRenderTypeOpenGL);
    auto wnd = (CWndUI*)(paint_manager_.FindControl("renderWindow"));
    wnd->SetEnabled(false);
    wnd->SetVisible(true);
    wnd->SetPos({0, 0, 900, 500});
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
                if (frame_queue_.empty()) {
                    continue;
                }
                frame = frame_queue_.front();
                frame_queue_.pop();
            }
            uint32_t width = 1280;
            uint32_t height = 720;
            uint8_t* y_data = frame->GetData();
            uint8_t* u_data = y_data + width * height;
            uint8_t* v_data = y_data + width * height * 5 / 4;
            video_render_->RendFrameI420(y_data, width, u_data, width / 2, v_data, width / 2, width,
                                         height);
        }
    });
}