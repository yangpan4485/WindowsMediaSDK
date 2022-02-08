#pragma once
#include "UIlib.h"

class CWndUI : public DuiLib::CControlUI {
public:
    CWndUI();
    virtual void SetInternVisible(bool bVisible = false);
    virtual void SetPos(RECT rc);

    BOOL Attach(HWND hWndNew);
    HWND Detach();
    HWND GetHwnd();

protected:
    HWND m_hWnd;
};
