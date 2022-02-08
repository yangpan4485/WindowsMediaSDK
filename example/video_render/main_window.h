#pragma once
#include <memory>
#include <thread>

#include "UIlib.h"
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

private:
    LPCTSTR GetWindowClassName() const override;
    void Notify(DuiLib::TNotifyUI& msg) override;
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnClick(DuiLib::TNotifyUI& msg);
    DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass) override;

private:
    DuiLib::CPaintManagerUI paint_manager_{};
    HINSTANCE hinstance_{};
    std::shared_ptr<VideoRender> video_render_{};
    std::thread render_thread_{};
    bool running_{false};
};
