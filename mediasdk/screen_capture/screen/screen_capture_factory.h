// #pragma once
#include <memory>
#include "screen_capture.h"

enum CaptureType {
    kCaptureTypeDefault = 0,
    kCaptureTypeMag = 1,
    kCaptureTypeGDI = 2,
    kCaptureTypeDX = 3,
};

class ScreenCaptureFactory {
public:
    static ScreenCaptureFactory& GetInstance();
    std::shared_ptr<ScreenCapture> CreateScreenCapture(CaptureType type, const CaptureConfig& config);
    std::shared_ptr<ScreenCapture> CreateWindowCapture(CaptureType type);

private:
    ScreenCaptureFactory();
    ~ScreenCaptureFactory();
    ScreenCaptureFactory(const ScreenCaptureFactory&) = delete;
    ScreenCaptureFactory operator=(const ScreenCaptureFactory&) = delete;
};