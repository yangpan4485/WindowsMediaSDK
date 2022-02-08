#include "main_window.h"

#include <fstream>
#include <iostream>
#include <string>

#include <VersionHelpers.h>
#include <shellscalingapi.h>

#include "my_window.h"
#include "video_render_factory.h"

#pragma comment(lib, "Shcore.lib")

MainWindow::MainWindow() {}

MainWindow::~MainWindow() {}

void MainWindow::Init() {
    if (IsWindows8OrGreater()) {
        SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    }
    hinstance_ = GetModuleHandle(0);
    DuiLib::CPaintManagerUI::SetInstance(hinstance_);
    std::string path = "..\\..\\..\\example\\video_render\\xml";
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
    running_ = false;
    if (render_thread_.joinable()) {
        render_thread_.join();
    }
    return 0;
}

LRESULT MainWindow::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return 0;
}

void MainWindow::OnClick(DuiLib::TNotifyUI& msg) {
    running_ = false;
    if (render_thread_.joinable()) {
        render_thread_.join();
    }

    auto name = msg.pSender->GetName();
    if (name == "btnQuit") {
        Close();
        return;
    } else if (name == "btnD3D9") {
        video_render_ = VideoRenderFactory::CreateInstance()->CreateVideoRender(kRenderTypeD3D9);
    } else if (name == "btnSDL") {
        video_render_ = VideoRenderFactory::CreateInstance()->CreateVideoRender(kRenderTypeSDL);
    } else if (name == "btnOpenGL") {
        std::cout << "btnOpenGL" << std::endl;
        video_render_ = VideoRenderFactory::CreateInstance()->CreateVideoRender(kRenderTypeOpenGL);
    }
    if (!video_render_) {
        return;
    }
    auto wnd = (CWndUI*)(paint_manager_.FindControl("renderWindow"));
    wnd->SetEnabled(false);
    wnd->SetVisible(true);
    wnd->SetPos({0, 0, 900, 500});
    ::ShowWindow(wnd->GetHwnd(), true);
    HWND hwnd = wnd->GetHwnd();
    video_render_->SetWindow(hwnd);
    running_ = true;
    render_thread_ = std::thread([&]() {
        std::ifstream fin("../../../1280_720.yuv",
                          std::ios::binary | std::ios::in);
        if (!fin) {
            std::cout << "open file failed" << std::endl;
            return;
        }
        uint32_t width = 1280;
        uint32_t height = 720;
        uint8_t* y_data = new uint8_t[width * height];
        uint8_t* u_data = new uint8_t[width * height / 4];
        uint8_t* v_data = new uint8_t[width * height / 4];
        while (running_) {
            if (fin.eof()) {
                break;
                // fin.seekg(ios::beg);
            }
            fin.read((char*)y_data, width * height);
            fin.read((char*)u_data, width * height / 4);
            fin.read((char*)v_data, width * height / 4);
            video_render_->RendFrameI420(y_data, width, u_data, width / 2, v_data, width / 2, width,
                                         height);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        std::cout << "end" << std::endl;
        fin.close();
        delete[] y_data;
        delete[] u_data;
        delete[] v_data;
    });
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
