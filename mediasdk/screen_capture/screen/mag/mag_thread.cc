#include "mag_thread.h"

#include <magnification.h>

constexpr UINT WM_CREATE_MAG = WM_USER + 1;
constexpr UINT WM_DESTROY_MAG = WM_USER + 2;

static LRESULT CALLBACK MessageOnlyWinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MagThread* magThread = nullptr;
    if (uMsg == WM_NCCREATE) {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        magThread = static_cast<MagThread*>(lpcs->lpCreateParams);
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(magThread));
    }
    else {
        magThread = reinterpret_cast<MagThread*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (uMsg == WM_CLOSE && magThread != nullptr) {
            DestroyWindow(hWnd);
            return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
        if (uMsg == WM_NCDESTROY && magThread != nullptr) {
            ::PostQuitMessage(0);
            return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }

    if (magThread == nullptr) {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    LRESULT lResult{};
    if (!magThread->HandleMessage(uMsg, wParam, lParam, lResult)) {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return lResult;
}

MagThread& MagThread::GetInstance() {
    static MagThread instance;
    return instance;
}

bool MagThread::Init() {
    if (m_initialize) {
        return true;
    }
    m_initializeEvent = CreateEvent(nullptr, true, false, nullptr);
    mag_thread_ = std::thread(&MagThread::threadProc, this);
    WaitForSingleObject(m_initializeEvent, INFINITE);
    m_initialize = true;
    CloseHandle(m_initializeEvent);
    m_initializeEvent = nullptr;
    return true;
}

MagThread::MagThread() {

}

MagThread::~MagThread() {
    if (mag_thread_.joinable()) {
        mag_thread_.join();
    }
}

void MagThread::threadProc() {
    // CoInitialize(nullptr);
    // CoInitializeEx(NULL, COINIT_MULTITHREADED);
    createMessageWindow();
    MagInitialize();
    SetEvent(m_initializeEvent);
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_QUIT) {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    MagUninitialize();
    CoUninitialize();
}

void MagThread::createMessageWindow() {
    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = (WNDPROC)MessageOnlyWinProc;
    wc.lpszClassName = L"VCMessageWindow";
    RegisterClassExW(&wc);
    m_messageWnd = CreateWindowExW(0, L"VCMessageWindow", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, 0, 0, this);
}

bool MagThread::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult) {
    if (uMsg == WM_CREATE_MAG) {
        MagWindowParam* param = reinterpret_cast<MagWindowParam*>(wParam);
        MagWindow* magWin = createMagWindow(*param);
        lResult = reinterpret_cast<LRESULT>(magWin);
        return true;
    }

    if (uMsg == WM_DESTROY_MAG) {
        MagWindow* magWin = reinterpret_cast<MagWindow*>(wParam);
        destroyMagWindow(magWin);
        lResult = 0;
        return true;
    }
    return false;
}

MagWindow* MagThread::createMagWindow(const MagWindowParam& param) {
    MagWindow* magWindow = new MagWindow();
    if (!magWindow->CreateMagWindow(param)) {
        delete magWindow;
        return nullptr;
    }

    return magWindow;
}

void MagThread::destroyMagWindow(MagWindow* magWindow) {
    magWindow->DestroyMagWindow();
    delete magWindow;
}

MagWindow* MagThread::CreateMagWindowOnThread(const MagWindowParam& param) {
    LRESULT ret =
        SendMessage(m_messageWnd, WM_CREATE_MAG, reinterpret_cast<WPARAM>(&param), reinterpret_cast<LPARAM>(nullptr));
    return reinterpret_cast<MagWindow*>(ret);
}

void MagThread::DestroyMagWindowOnThread(MagWindow* magWindow) {
    SendMessage(m_messageWnd, WM_DESTROY_MAG, reinterpret_cast<WPARAM>(magWindow), reinterpret_cast<LPARAM>(nullptr));
}