#pragma once
#include <memory>
#include <thread>

#include "UIlib.h"
class SocketServer;
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
    void MoveButton();

private:
    DuiLib::CPaintManagerUI paint_manager_{};
    HINSTANCE hinstance_{};
    
    // std::thread render_thread_{};
    bool running_{false};
	std::shared_ptr<SocketServer> socket_serevr_{};
    uint32_t render_window_width_{900};
    uint32_t render_window_height_{500};
    bool init_ = false;
};
