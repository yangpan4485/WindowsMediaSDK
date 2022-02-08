#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <memory>
#include <vector>
#include <wrl/client.h>

#include "draw_cursor.h"
#include "screen/screen_capture.h"

using Microsoft::WRL::ComPtr;

class DXGITexture;
class ScreenCaptureDX : public ScreenCapture {
public:
    ScreenCaptureDX();
    ~ScreenCaptureDX();
    std::shared_ptr<VideoFrame> CaptureScreen(const ScreenInfo& screen_info) override;

private:
    bool Init(int display_id);
    bool Init();
    bool Uninit();
    bool Release();
    bool InitializeDXGI();
    std::shared_ptr<VideoFrame> GetFrame();
    bool SetDisplayId(uint32_t display_id);
    std::vector<ComPtr<IDXGIAdapter>> EnumAdapters();

private:
    Microsoft::WRL::ComPtr<ID3D11Device> d3d_device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<IDXGIDevice> dxgi_device_;
    Microsoft::WRL::ComPtr<IDXGIOutput1> output_;
    Microsoft::WRL::ComPtr<IDXGIOutputDuplication> duplication_;

    std::vector<uint8_t> cursor_buffer_;
    DXGI_OUTDUPL_POINTER_SHAPE_INFO pointer_info_ = {0};
    DXGI_OUTDUPL_POINTER_POSITION current_pointer_positon_;
    bool init_{};

    std::shared_ptr<DXGITexture> dxgi_texture_{};
    DrawCursor draw_cursor_{};
};