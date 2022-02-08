#include "audio_device_window.h"

#include <iostream>

#include "string_utils.h"

AudioDeviceWindow::AudioDeviceWindow() {}

AudioDeviceWindow::~AudioDeviceWindow() {}

LPCTSTR AudioDeviceWindow::GetWindowClassName() const {
    return _T("DUIAudioDeviceFrame");
}

UINT AudioDeviceWindow::GetClassStyle() const {
    return UI_CLASSSTYLE_DIALOG;
}

void AudioDeviceWindow::Notify(DuiLib::TNotifyUI& msg) {
    auto name = msg.pSender->GetName();
    if (msg.sType == _T("click")) {
        if (name == "btnOk") {
            /*for (size_t i = 0; i < option_vec_.size(); ++i) {
                auto option = dynamic_cast<DuiLib::COptionUI*>(
                    paint_manager_.FindControl(option_vec_[i].c_str()));
                if (option && option->IsSelected()) {
                    select_id_ = video_devices_[i].device_id;
                    if (callback_) {
                        callback_(select_id_);
                    }
                    break;
                }
            }*/
            Close();
        }
    }
}

LRESULT AudioDeviceWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
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

LRESULT AudioDeviceWindow::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) {
    return 0;
}

LRESULT AudioDeviceWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    paint_manager_.Init(m_hWnd);
    paint_manager_.AddPreMessageFilter(this);
    DuiLib::CDialogBuilder builder;
    DuiLib::CControlUI* pRoot =
        builder.Create(_T("audio_device_window.xml"), (UINT)0, NULL, &paint_manager_);
    paint_manager_.AttachDialog(pRoot);
    paint_manager_.AddNotifier(this);
    InitWindow();
    return 0;
}

LRESULT AudioDeviceWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return 0;
}

void AudioDeviceWindow::InitWindow() {
    if (playout_audio_devices_.empty() && record_audio_devices_.empty()) {
        return;
    }

    auto device_window =
        dynamic_cast<DuiLib::CVerticalLayoutUI*>(paint_manager_.FindControl(_T("deviceWindow")));
    LONG startX = 10;
    LONG startY = 5;
    LONG width = 150;
    LONG height = 30;
    DuiLib::CLabelUI* inputLabel = new DuiLib::CLabelUI;
    inputLabel->SetText(_T("扬声器"));
    inputLabel->SetName(_T("playbackAudioSwitch"));
    inputLabel->SetFloat(true);
    inputLabel->SetFont(0);
    SIZE leftTop = {startX, startY};
    inputLabel->SetFixedXY(leftTop);
    inputLabel->SetFixedWidth(width);
    inputLabel->SetFixedHeight(height);
    device_window->Add(inputLabel);

    std::string device_name = "";
    playback_vec_.swap(std::vector<std::string>());
    for (size_t i = 0; i < playout_audio_devices_.size(); ++i) {
        device_name = "playback_audio_";
        DuiLib::COptionUI* option = new DuiLib::COptionUI;
        DuiLib::CLabelUI* label = new DuiLib::CLabelUI;
        label->SetText(utils::Utf8ToAscii(playout_audio_devices_[i].device_name).c_str());
        device_name = device_name + std::to_string(i);
        option->SetName(device_name.c_str());
        playback_vec_.push_back(device_name);
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
        option->SetGroup(_T("playbackGroup"));
        if (playback_select_id_.empty() && i == 0) {
            option->Selected(true);
        } else if (playback_select_id_ == playout_audio_devices_[i].device_id) {
            option->Selected(true);
        }
        device_window->Add(option);
        device_window->Add(label);
    }

    startY = startY + 55;
    DuiLib::CControlUI* control = new DuiLib::CControlUI;
    control->SetFloat(true);
    leftTop = {startX, startY};
    control->SetFixedXY(leftTop);
    // RECT
    control->SetFixedWidth(580);
    control->SetFixedHeight(1);
    control->SetBkColor(0XFF000000);
    device_window->Add(control);

    startY = startY + 25;
    DuiLib::CLabelUI* recordLabel = new DuiLib::CLabelUI;
    recordLabel->SetText(_T("麦克风"));
    recordLabel->SetName(_T("playbackAudioSwitch"));
    recordLabel->SetFloat(true);
    recordLabel->SetFont(0);
    leftTop = {startX, startY};
    recordLabel->SetFixedXY(leftTop);
    recordLabel->SetFixedWidth(width);
    recordLabel->SetFixedHeight(height);
    device_window->Add(recordLabel);

    record_vec_.swap(std::vector<std::string>());
    std::cout << "record_devices_ size:" << record_audio_devices_.size() << std::endl;
    for (size_t i = 0; i < record_audio_devices_.size(); ++i) {
        device_name = "record_audio_";
        DuiLib::COptionUI* option = new DuiLib::COptionUI;
        DuiLib::CLabelUI* label = new DuiLib::CLabelUI;
        label->SetText(utils::Utf8ToAscii(record_audio_devices_[i].device_name).c_str());
        device_name = device_name + std::to_string(i);
        option->SetName(device_name.c_str());
        record_vec_.push_back(device_name);
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
        option->SetGroup(_T("recordGroup"));
        if (record_select_id_.empty() && i == 0) {
            option->Selected(true);
        } else if (record_select_id_ == record_audio_devices_[i].device_id) {
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

void AudioDeviceWindow::SetAudioDevies(const std::vector<AudioDeviceInfo>& playout_audio_devices,
                                       const std::vector<AudioDeviceInfo>& record_audio_devices) {
    playout_audio_devices_ = playout_audio_devices;
    record_audio_devices_ = record_audio_devices;
}