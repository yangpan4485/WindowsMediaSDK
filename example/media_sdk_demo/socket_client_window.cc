#include "socket_client.h"
#include "socket_client_window.h"

#include <fstream>
#include <string>

SocketClientWindow::SocketClientWindow() : socket_client_(new SocketClient()) {}

SocketClientWindow::~SocketClientWindow() {
	DestroyWindow(GetParent(this->GetHWND()));
}

LPCTSTR SocketClientWindow::GetWindowClassName() const {
	return _T("DUISocketClientFrame");
}

UINT SocketClientWindow::GetClassStyle() const {
	return UI_CLASSSTYLE_DIALOG;
}

void SocketClientWindow::Notify(DuiLib::TNotifyUI& msg) {
	auto name = msg.pSender->GetName();
	if (msg.sType == _T("click")) {
		if (name == "btnConnect") {
			if (is_connect_) {
				return;
			}
			auto ip_edit = (DuiLib::CEditUI*)paint_manager_.FindControl(_T("ipEdit"));
			auto port_edit = (DuiLib::CEditUI*)paint_manager_.FindControl(_T("portEdit"));
			std::string ip = ip_edit->GetText();
			std::string port = port_edit->GetText();
			if (socket_client_->ConnectServer(ip, std::stoi(port))) {
				is_connect_ = true;
			}
			else {
				is_connect_ = false;
			}
            if (is_connect_) {
                Close();
            }
		}
		else if (name == "btnClose") {
			Close();
		}
	}
}

LRESULT SocketClientWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
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

LRESULT SocketClientWindow::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) {
	return 0;
}

LRESULT SocketClientWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	paint_manager_.Init(m_hWnd);
	paint_manager_.AddPreMessageFilter(this);
	DuiLib::CDialogBuilder builder;
	DuiLib::CControlUI* pRoot =
		builder.Create(_T("socket_client_window.xml"), (UINT)0, NULL, &paint_manager_);
	paint_manager_.AttachDialog(pRoot);
	paint_manager_.AddNotifier(this);
	InitWindow();
	return 0;
}

LRESULT SocketClientWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    std::ofstream fout("../../../config.txt");
    if (!fout || !fout.is_open()) {
        return 0;
    }
    std::string ip, port;
    auto ip_edit = (DuiLib::CEditUI*)(paint_manager_.FindControl(_T("ipEdit")));
    auto port_edit = (DuiLib::CEditUI*)(paint_manager_.FindControl(_T("portEdit")));
    if (ip_edit) {
        ip = ip_edit->GetText();
        ip = ip + "\n";
    }
    if (port_edit) {
        port = port_edit->GetText();
        port = port + "\n";
    }
    fout.write(ip.c_str(), ip.length());
    fout.write(port.c_str(), port.length());
    fout.close();
	return 0;
}

void SocketClientWindow::InitWindow() {
    std::ifstream fin("../../../config.txt");
    if (!fin || !fin.is_open()) {
        return;
    }
    std::string ip, port;
    std::getline(fin, ip);
    std::getline(fin, port);
    fin.close();
    auto ip_edit = (DuiLib::CEditUI*)(paint_manager_.FindControl(_T("ipEdit")));
    auto port_edit = (DuiLib::CEditUI*)(paint_manager_.FindControl(_T("portEdit")));
    if (ip_edit) {
        ip_edit->SetText(ip.c_str());
    }
    if (port_edit) {
        port_edit->SetText(port.c_str());
    }
}

bool SocketClientWindow::IsConnect() {
	return is_connect_;
}

void SocketClientWindow::SendFrame(uint8_t* data, uint32_t size) {
	if (socket_client_) {
		socket_client_->SendSocketMessage(data, size);
	}
}