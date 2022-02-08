#pragma once
#include <memory>
#include <thread>

#include "UIlib.h"
#include "event_callback.h"
#include "screen_capture_engine.h"

class MainWindow : public DuiLib::CWindowWnd, public DuiLib::INotifyUI {
public:
    MainWindow();
    ~MainWindow();

    void Init();
    void CreateDuiWindow();
    void Show();
    void OnScreenEvent(ScreenEvent screen_event);

private:
    LPCTSTR GetWindowClassName() const override;
    void Notify(DuiLib::TNotifyUI& msg) override;
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnClick(DuiLib::TNotifyUI& msg);

    HBITMAP GetBitmap(const std::string& image);
    void GetScreenShareList();
    void StartCaptureScreen(int index);
    void StartCaptureWindow(int index);
    void StopCapture();

private:
    DuiLib::CPaintManagerUI paint_manager_{};
    HINSTANCE hinstance_{};
    DuiLib::CTileLayoutUI* thumbnail_show_window_list_{};
    ScreenCaptureEngine screen_capture_engine_{};
    std::shared_ptr<ICaptureHandler> capture_handler_{};
    std::vector<ScreenInfo> screen_list_{};
    std::vector<WindowInfo> window_list_{};
    std::thread work_thread_{};
    std::string select_name_{};
    std::shared_ptr<EventCallback> event_callback_{};
};
