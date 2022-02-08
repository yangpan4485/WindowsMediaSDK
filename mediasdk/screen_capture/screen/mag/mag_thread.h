#pragma once
#include <Windows.h>
#include <thread>

#include "mag_window.h"
class MagThread {
public:
    static MagThread& GetInstance();

    bool Init();
    void threadProc();
    void createMessageWindow();
    bool HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult);
    MagWindow* createMagWindow(const MagWindowParam& param);
    void destroyMagWindow(MagWindow* magWindow);

    MagWindow* CreateMagWindowOnThread(const MagWindowParam& param);
    void DestroyMagWindowOnThread(MagWindow* magWindow);

private:
    MagThread();
    ~MagThread();

    MagThread(const MagThread&) = delete;
    MagThread operator=(const MagThread&) = delete;

private:
    bool m_initialize{};
    HANDLE m_initializeEvent = nullptr;
    std::thread mag_thread_;
    HWND m_messageWnd;
};