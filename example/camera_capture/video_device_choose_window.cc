#include "video_device_choose_window.h"

#include "string_utils.h"

VideoDeviceWindow::VideoDeviceWindow() {}

VideoDeviceWindow::~VideoDeviceWindow() {
    DestroyWindow(GetParent(this->GetHWND()));
}

LPCTSTR VideoDeviceWindow::GetWindowClassName() const {
    return _T("DUIVideoDeviceFrame");
}

UINT VideoDeviceWindow::GetClassStyle() const {
    return UI_CLASSSTYLE_DIALOG;
}

void VideoDeviceWindow::Notify(DuiLib::TNotifyUI& msg) {
    auto name = msg.pSender->GetName();
    if (msg.sType == _T("click")) {
        if (name == "btnOk") {
            for (size_t i = 0; i < option_vec_.size(); ++i) {
                auto option = dynamic_cast<DuiLib::COptionUI*>(
                    paint_manager_.FindControl(option_vec_[i].c_str()));
                if (option && option->IsSelected()) {
                    select_id_ = video_devices_[i].device_id;
                    if (callback_) {
                        callback_(select_id_);
                    }
                    break;
                }
            }
            Close();
        }
    }
}

LRESULT VideoDeviceWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
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

LRESULT VideoDeviceWindow::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) {
    return 0;
}

LRESULT VideoDeviceWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    paint_manager_.Init(m_hWnd);
    paint_manager_.AddPreMessageFilter(this);
    DuiLib::CDialogBuilder builder;
    DuiLib::CControlUI* pRoot =
        builder.Create(_T("video_device_window.xml"), (UINT)0, NULL, &paint_manager_);
    paint_manager_.AttachDialog(pRoot);
    paint_manager_.AddNotifier(this);
    InitWindow();
    return 0;
}

LRESULT VideoDeviceWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return 0;
}

void VideoDeviceWindow::InitWindow() {
    if (video_devices_.empty()) {
        return;
    }

    auto device_window =
        dynamic_cast<DuiLib::CVerticalLayoutUI*>(paint_manager_.FindControl(_T("deviceWindow")));
    LONG startX = 10;
    LONG startY = 5;
    LONG width = 150;
    LONG height = 30;
    DuiLib::CLabelUI* inputLabel = new DuiLib::CLabelUI;
    inputLabel->SetText(_T("视频设备选择"));
    inputLabel->SetName(_T("videoSwitch"));
    inputLabel->SetFloat(true);
    inputLabel->SetFont(0);
    SIZE leftTop = {startX, startY};
    inputLabel->SetFixedXY(leftTop);
    inputLabel->SetFixedWidth(width);
    inputLabel->SetFixedHeight(height);
    device_window->Add(inputLabel);

    std::string device_name = "";
    option_vec_.swap(std::vector<std::string>());
    for (size_t i = 0; i < video_devices_.size(); ++i) {
        device_name = "camera_";
        DuiLib::COptionUI* option = new DuiLib::COptionUI;
        DuiLib::CLabelUI* label = new DuiLib::CLabelUI;
        label->SetText(utils::Utf8ToAscii(video_devices_[i].device_name).c_str());
        device_name = device_name + std::to_string(i);
        option->SetName(device_name.c_str());
        option_vec_.push_back(device_name);
        option->SetFloat(true);
        label->SetFloat(true);
        option->SetFont(0);
        label->SetFont(0);
        startY = startY + 40;
        SIZE leftTop = {startX, startY};
        option->SetFixedXY(leftTop);
        option->SetFixedWidth(20);
        option->SetFixedHeight(20);
        label->SetFixedXY({startX + 22, startY});
        label->SetFixedWidth(580);
        label->SetFixedHeight(20);
        option->SetNormalImage(_T("..\\..\\resources\\common\\radio_un.png"));
        option->SetSelectedImage(_T("..\\..\\resources\\common\\radio_sel.png"));
        option->SetGroup(_T("cameraGroup"));
        if ((select_id_.empty() || select_id_ == "auto") && i == 0) {
            option->Selected(true);
        } else if (select_id_ == video_devices_[i].device_id) {
            option->Selected(true);
        }
        device_window->Add(option);
        device_window->Add(label);
    }
    RECT rect;
    GetWindowRect(m_hWnd, &rect);
    int nwidth = (rect.right - rect.left);
    startX = nwidth - 120;
    startY = startY + 40;
    DuiLib::CButtonUI* button = new DuiLib::CButtonUI;
    button->SetText(_T("确定"));
    button->SetName(_T("btnOk"));
    button->SetFloat(true);
    button->SetFont(0);
    button->SetPushedImage("..\\..\\resources\\common\\button_pushed.png");
    button->SetNormalImage("..\\..\\resources\\common\\button_normal.png");
    button->SetFocusedImage("..\\..\\resources\\common\\button_hover.png");
    leftTop = {startX, startY};
    button->SetFixedXY(leftTop);
    button->SetFixedWidth(100);
    button->SetFixedHeight(height);
    device_window->Add(button);
}

void VideoDeviceWindow::SetCurrentVideoDevice(const std::string& device_id) {
    select_id_ = device_id;
}

void VideoDeviceWindow::SetVideoDevies(const std::vector<VideoDeviceInfo>& video_devices) {
    video_devices_.swap(std::vector<VideoDeviceInfo>(video_devices.begin(), video_devices.end()));
}

void VideoDeviceWindow::SetVideoDeviceCallback(VideoDeviceCallback callback) {
    callback_ = callback;
}