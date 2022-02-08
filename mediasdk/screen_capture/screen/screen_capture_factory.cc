#include "screen_capture_factory.h"

#include "screen/dxgi/screen_capture_dxgi.h"
#include "screen/gdi/screen_capture_gdi.h"
#include "screen/mag/screen_capture_mag.h"

ScreenCaptureFactory& ScreenCaptureFactory::GetInstance() {
    static ScreenCaptureFactory instance;
    return instance;
}

std::shared_ptr<ScreenCapture> ScreenCaptureFactory::CreateScreenCapture(CaptureType type, const CaptureConfig& config) {
    std::shared_ptr<ScreenCapture> screen_capture = nullptr;
    if (type == kCaptureTypeMag) {
        screen_capture = std::make_shared<ScreenCaptureMag>();
    } else if (type == kCaptureTypeGDI) {
        screen_capture = std::make_shared<ScreenCaptureGDI>();
    } else if (type == kCaptureTypeDX) {
        screen_capture = std::make_shared<ScreenCaptureDX>();
    }
    return screen_capture;
}

std::shared_ptr<ScreenCapture> ScreenCaptureFactory::CreateWindowCapture(CaptureType type) {
    // window capture
    std::shared_ptr<ScreenCapture> screen_capture = nullptr;
    if (type == kCaptureTypeMag) {
        screen_capture = std::make_shared<ScreenCaptureMag>();
    } else if (type == kCaptureTypeGDI) {
        screen_capture = std::make_shared<ScreenCaptureGDI>();
    }
    return screen_capture;
}

ScreenCaptureFactory::ScreenCaptureFactory() {}

ScreenCaptureFactory::~ScreenCaptureFactory() {}