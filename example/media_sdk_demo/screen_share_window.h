#pragma once
#include <functional>
#include <memory>
#include <thread>

#include "UIlib.h"
#include "screen_capture_engine.h"

class ScreenShareWindow : public DuiLib::CWindowWnd, public DuiLib::INotifyUI {
public:
    using ScreenWindowCallback = std::function<void(const ScreenInfo& screen_info,
                                                    const WindowInfo& window_info, bool is_screen)>;

public:
    ScreenShareWindow();
    ~ScreenShareWindow();

    void CreateDuiWindow();
    void Show();
    void SetScreenShareEngine(std::shared_ptr<ScreenCaptureEngine> engine);
    void SetScreenWindowCallback(ScreenWindowCallback callback);

private:
    LPCTSTR GetWindowClassName() const override;
    void Notify(DuiLib::TNotifyUI& msg) override;
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnClick(DuiLib::TNotifyUI& msg);
    void InitWindow();
    HBITMAP GetBitmap(const std::string& image);

private:
    DuiLib::CPaintManagerUI paint_manager_{};
    HINSTANCE hinstance_{};
    std::weak_ptr<ScreenCaptureEngine> screen_capture_engine_{};
    std::thread work_thread_{};
    bool running_{};
    DuiLib::CTileLayoutUI* thumbnail_show_window_list_{};
    std::vector<ScreenInfo> screen_list_{};
    std::vector<WindowInfo> window_list_{};
    std::string select_name_{};
    ScreenWindowCallback callback_{};
};