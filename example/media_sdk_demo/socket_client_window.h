#pragma once
#include <functional>
#include <string>

#include "UIlib.h"
class SocketClient;
class SocketClientWindow : public DuiLib::CWindowWnd,
	public DuiLib::INotifyUI,
	public DuiLib::IMessageFilterUI {
public:
	SocketClientWindow();
	~SocketClientWindow();

	LPCTSTR GetWindowClassName() const override;
	UINT GetClassStyle() const override;
	void Notify(DuiLib::TNotifyUI& msg) override;
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool IsConnect();
	void SendFrame(uint8_t* data, uint32_t size);

private:
	void InitWindow();

private:
	DuiLib::CPaintManagerUI paint_manager_{};
	bool is_connect_{};
	std::shared_ptr<SocketClient> socket_client_{};
};