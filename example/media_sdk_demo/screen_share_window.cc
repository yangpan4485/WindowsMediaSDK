#include "screen_share_window.h"

#include "Utils/stb_image.h"
#include "string_utils.h"
#include <iostream>

const std::string window_name = "window_";
const std::string screen_name = "screen_";

ScreenShareWindow::ScreenShareWindow() {}

ScreenShareWindow::~ScreenShareWindow() {
    running_ = false;
    if (work_thread_.joinable()) {
        work_thread_.join();
    }
}

void ScreenShareWindow::CreateDuiWindow() {
    Create(NULL, _T("ScreenShareWindow"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
}

void ScreenShareWindow::Show() {
    ShowModal();
}

LPCTSTR ScreenShareWindow::GetWindowClassName() const {
    return _T("DUIScreenShareFrame");
}

void ScreenShareWindow::Notify(DuiLib::TNotifyUI& msg) {
    if (msg.sType == _T("click")) {
        OnClick(msg);
    }
}

LRESULT ScreenShareWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LRESULT lRes = 0;
    switch (uMsg) {
    case WM_CREATE:
        lRes = OnCreate(uMsg, wParam, lParam);
        break;
    case WM_CLOSE:
        lRes = OnClose(uMsg, wParam, lParam);
        break;
    }
    if (paint_manager_.MessageHandler(uMsg, wParam, lParam, lRes)) {
        return lRes;
    }
    return __super::HandleMessage(uMsg, wParam, lParam);
}

LRESULT ScreenShareWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    paint_manager_.Init(m_hWnd);
    DuiLib::CDialogBuilder builder;
    DuiLib::CControlUI* pRoot =
        builder.Create(_T("screen_share_window.xml"), (UINT)0, NULL, &paint_manager_);
    paint_manager_.AttachDialog(pRoot);
    paint_manager_.AddNotifier(this);
    InitWindow();
    return 0;
}

LRESULT ScreenShareWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return 0;
}

void ScreenShareWindow::OnClick(DuiLib::TNotifyUI& msg) {
    std::string name = msg.pSender->GetName();
    if (name.find(window_name) != std::string::npos ||
        name.find(screen_name) != std::string::npos) {
        select_name_ = name;
    }
    if (name == "btnOk") {
        std::cout << "select name: " << select_name_ << std::endl;
        if (select_name_.empty()) {
            Close();
            return;
        }
        auto pos = select_name_.find(screen_name);
        bool share_screen = true;
        if (pos == std::string::npos) {
            pos = select_name_.find(window_name);
            if (pos == std::string::npos) {
                Close();
                return;
            }
            share_screen = false;
        }
        std::string str_index = select_name_.substr(7, select_name_.length());
        int index = std::stoi(str_index);
        if (callback_) {
            if (share_screen) {
                callback_(screen_list_[index - 1], WindowInfo(), true);
            } else {
                callback_(ScreenInfo(), window_list_[index - 1], false);
            }
        }
        Close();
    }
}

void ScreenShareWindow::InitWindow() {
    running_ = false;
    if (work_thread_.joinable()) {
        work_thread_.join();
    }
    running_ = true;
    work_thread_ = std::thread([&]() {
        thumbnail_show_window_list_ =
            (DuiLib::CTileLayoutUI*)(paint_manager_.FindControl(_T("windowList")));
        thumbnail_show_window_list_->RemoveAll();
        paint_manager_.RemoveAllImages(true);
        auto screen_capture_engine = screen_capture_engine_.lock();
        screen_list_ = screen_capture_engine->GetScreenLists();
        std::cout << "screen size: " << screen_list_.size() << std::endl;
        window_list_ = screen_capture_engine->GetWindowLists();
        std::cout << "window size: " << window_list_.size() << std::endl;
        int screen_count = screen_list_.size();
        int window_count = window_list_.size();
        for (int i = 0; i < screen_count; ++i) {
            if (!running_) {
                break;
            }
            std::string name = screen_name + std::to_string(i + 1);
            DuiLib::COptionUI* window_item = new DuiLib::COptionUI;
            window_item->SetNormalImage("..\\..\\resources\\common\\btn_nor.png");
            window_item->SetHotImage("..\\..\\resources\\common\\btn_hover.png");
            window_item->SetSelectedImage("..\\..\\resources\\common\\btn_sel.png");
            window_item->SetFixedWidth(270);
            window_item->SetFixedHeight(170);
            window_item->SetTextPadding({0, 120, 0, 0});
            window_item->SetName(name.c_str());
            std::string image_data =
                screen_capture_engine->GetScreenThumbImage(screen_list_[i], 640, 480);
            HBITMAP bitmap = GetBitmap(image_data);
            paint_manager_.AddImage(name.c_str(), bitmap, 640, 480, true, true);
            std::string forename = "file='" + name + "' dest='40,20,230,120'";
            window_item->SetForeImage(forename.c_str());
            std::string title = "Display " + std::to_string(i + 1);
            window_item->SetToolTip(title.c_str());
            if (title.length() > 20) {
                title = title.substr(0, 20) + "...";
            }
            window_item->SetText(title.c_str());
            window_item->SetFont(0);
            window_item->SetGroup("allWindow");
            thumbnail_show_window_list_->Add(window_item);
        }

        for (int i = 0; i < window_count; ++i) {
            if (!running_) {
                break;
            }
            if (window_list_[i].window_title == "Cortana") {
                continue;
            }
            std::string name = window_name + std::to_string(i + 1);
            DuiLib::COptionUI* window_item = new DuiLib::COptionUI;
            window_item->SetNormalImage("..\\..\\resources\\common\\btn_nor.png");
            window_item->SetHotImage("..\\..\\resources\\common\\btn_hover.png");
            window_item->SetSelectedImage("..\\..\\resources\\common\\btn_sel.png");
            window_item->SetFixedWidth(270);
            window_item->SetFixedHeight(170);
            window_item->SetTextPadding({0, 120, 0, 0});
            window_item->SetName(name.c_str());
            std::string image_data =
                screen_capture_engine->GetWindowThumbImage(window_list_[i], 640, 480);
            HBITMAP bitmap = GetBitmap(image_data);
            paint_manager_.AddImage(name.c_str(), bitmap, 640, 480, true, true);
            std::string forename = "file='" + name + "' dest='40,20,230,120'";
            window_item->SetForeImage(forename.c_str());
            std::string title = utils::Utf8ToAnsi(window_list_[i].window_title);
            window_item->SetToolTip(title.c_str());
            if (title.length() > 20) {
                title = title.substr(0, 20) + "...";
            }
            window_item->SetText(title.c_str());
            window_item->SetFont(0);
            window_item->SetGroup("allWindow");
            thumbnail_show_window_list_->Add(window_item);
        }
    });
}

void ScreenShareWindow::SetScreenShareEngine(std::shared_ptr<ScreenCaptureEngine> engine) {
    screen_capture_engine_ = engine;
}

HBITMAP ScreenShareWindow::GetBitmap(const std::string& image) {
    LPBYTE pImage = NULL;
    int x, y, n;
    pImage = stbi_load_from_memory((LPBYTE)image.c_str(), image.length(), &x, &y, &n, 4);
    if (!pImage) {
        return NULL;
    }
    BITMAPINFO bmi;
    ::ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = x;
    bmi.bmiHeader.biHeight = -y;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = x * y * 4;
    bool bAlphaChannel = false;
    LPBYTE pDest = NULL;
    HBITMAP hBitmap = ::CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&pDest, NULL, 0);
    if (!hBitmap) {
        return NULL;
    }
    for (int i = 0; i < x * y; i++) {
        pDest[i * 4 + 3] = pImage[i * 4 + 3];
        if (pDest[i * 4 + 3] < 255) {
            pDest[i * 4] = (BYTE)(DWORD(pImage[i * 4 + 2]) * pImage[i * 4 + 3] / 255);
            pDest[i * 4 + 1] = (BYTE)(DWORD(pImage[i * 4 + 1]) * pImage[i * 4 + 3] / 255);
            pDest[i * 4 + 2] = (BYTE)(DWORD(pImage[i * 4]) * pImage[i * 4 + 3] / 255);
            bAlphaChannel = true;
        } else {
            pDest[i * 4] = pImage[i * 4 + 2];
            pDest[i * 4 + 1] = pImage[i * 4 + 1];
            pDest[i * 4 + 2] = pImage[i * 4];
        }
        if (*(DWORD*)(&pDest[i * 4]) == 0) {
            pDest[i * 4] = (BYTE)0;
            pDest[i * 4 + 1] = (BYTE)0;
            pDest[i * 4 + 2] = (BYTE)0;
            pDest[i * 4 + 3] = (BYTE)0;
            bAlphaChannel = true;
        }
    }
    stbi_image_free(pImage);
    return hBitmap;
}

void ScreenShareWindow::SetScreenWindowCallback(ScreenWindowCallback callback) {
    callback_ = callback;
}
