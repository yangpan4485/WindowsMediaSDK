#include "screen_capture_engine.h"

#include "common/version_helper.h"
#include "screen/screen_capture.h"
#include "screen/screen_capture_factory.h"
#include "screen/window_manager.h"
#include "screen/window_utils.h"

class ScreenCaptureEngine::ScreenCaptureEngineImpl {
public:
    ScreenCaptureEngineImpl() {}
    ~ScreenCaptureEngineImpl() {}

    std::vector<ScreenInfo> GetScreenLists() {
        return window_manager_.GetScreenList();
    }

    std::vector<WindowInfo> GetWindowLists() {
        return window_manager_.GetWindowList();
    }

    std::string GetWindowThumbImage(const WindowInfo& window_info, uint32_t thumb_width,
                                    uint32_t thumb_height) {
        return window_manager_.GetWindowThumbImage(window_info.hwnd, thumb_width, thumb_height);
    }

    std::string GetScreenThumbImage(const ScreenInfo& screen_info, uint32_t thumb_width,
                                    uint32_t thumb_height) {
        return window_manager_.GetScreenThumbImage(screen_info, thumb_width, thumb_height);
    }

    void StartCaptureDisplay(const ScreenInfo& screen_info, const CaptureConfig& config,
                             const std::vector<HWND>& ignore_window) {
        if (screen_capture_) {
            return;
        }
        // CreateEngine By BitBlt Because Of Win10 integrateGPU=false
        // CreateEngine By BitBlt Because Of Win7 Basic
#if 1
        
        /*if (!screen_capture_) {
            screen_capture_ = ScreenCaptureFactory::GetInstance().CreateScreenCapture(kCaptureTypeMag);
        }*/
        // 一般不使用 DXGI
        if (!screen_capture_) {
            screen_capture_ = ScreenCaptureFactory::GetInstance().CreateScreenCapture(kCaptureTypeMag, config);
        }
#else
        CaptureType capture_type = kCaptureTypeDefault;
        if (VersionHelper::Instance().IsWindows7() && !AeroEnabled()) {
            capture_type = kCaptureTypeGDI;
        }
        HRESULT hr;
        bool intel_gpu = IsRunOnIntegrateGPU(hr);
        if (VersionHelper::Instance().IsWindows10OrLater() && !intel_gpu) {
            if (hr != E_OUTOFMEMORY) {
                capture_type = kCaptureTypeGDI;
            }

        }
        if (capture_type == kCaptureTypeDefault && config.hd_mode_) {
            capture_type = kCaptureTypeDX;
        }
        screen_capture_ =
            ScreenCaptureFactory::GetInstance().CreateScreenCapture(capture_type, config);
        if (!screen_capture_) {
            return;
        }
#endif
        screen_capture_->RegisterCaptureHandler(capture_handler_);
        screen_capture_->SetIgnoreWindowList(ignore_window);
        screen_capture_->StartCaptureDisplay(screen_info, config);
    }

    void StartCaptureWindow(const WindowInfo& window_info, const CaptureConfig& config) {
        if (screen_capture_) {
            return;
        }
#if 1
        screen_capture_ = ScreenCaptureFactory::GetInstance().CreateWindowCapture(kCaptureTypeMag);
#else
        CaptureType capture_type = kCaptureTypeDefault;
        if (VersionHelper::Instance().IsWindows7() && !AeroEnabled()) {
            capture_type = kCaptureTypeGDI;
        }
        HRESULT hr;
        bool intel_gpu = IsRunOnIntelGPU(hr);
        if (VersionHelper::Instance().IsWindows10OrLater() && !intel_gpu) {
            capture_type = kCaptureTypeGDI;
        }
        if (capture_type == kCaptureTypeDefault) {
            capture_type = kCaptureTypeMag;
        }
        screen_capture_ = ScreenCaptureFactory::GetInstance().CreateWindowCapture(capture_type);
        if (!screen_capture_) {
            return;
        }
#endif
        screen_capture_->RegisterCaptureHandler(capture_handler_);
        screen_capture_->StartCaptureWindow(window_info, config);
    }

    void StopCapture() {
        if (!screen_capture_) {
            return;
        }
        screen_capture_->StopCapture();
        screen_capture_.reset();
    }

    void RegisterCaptureHandler(std::shared_ptr<ICaptureHandler> handler) {
        capture_handler_ = handler;
    }

private:
    WindowManager window_manager_;
    std::shared_ptr<ScreenCapture> screen_capture_{};
    std::shared_ptr<ICaptureHandler> capture_handler_{};
};

ScreenCaptureEngine::ScreenCaptureEngine() : pimpl_(new ScreenCaptureEngineImpl()) {}

ScreenCaptureEngine::~ScreenCaptureEngine() {}

std::vector<ScreenInfo> ScreenCaptureEngine::GetScreenLists() {
    return pimpl_->GetScreenLists();
}

std::vector<WindowInfo> ScreenCaptureEngine::GetWindowLists() {
    return pimpl_->GetWindowLists();
}

std::string ScreenCaptureEngine::GetWindowThumbImage(const WindowInfo& window_info,
                                                     uint32_t thumb_width, uint32_t thumb_height) {
    return pimpl_->GetWindowThumbImage(window_info, thumb_width, thumb_height);
}

std::string ScreenCaptureEngine::GetScreenThumbImage(const ScreenInfo& screen_info,
                                                     uint32_t thumb_width, uint32_t thumb_height) {
    return pimpl_->GetScreenThumbImage(screen_info, thumb_width, thumb_height);
}

void ScreenCaptureEngine::StartCaptureDisplay(const ScreenInfo& screen_info,
                                              const CaptureConfig& config,
                                              const std::vector<HWND>& ignore_window) {
    pimpl_->StartCaptureDisplay(screen_info, config, ignore_window);
}

void ScreenCaptureEngine::StartCaptureWindow(const WindowInfo& window_info,
                                             const CaptureConfig& config) {
    pimpl_->StartCaptureWindow(window_info, config);
}

void ScreenCaptureEngine::RegisterCaptureHandler(std::shared_ptr<ICaptureHandler> handler) {
    pimpl_->RegisterCaptureHandler(handler);
}

void ScreenCaptureEngine::StopCapture() {
    pimpl_->StopCapture();
}