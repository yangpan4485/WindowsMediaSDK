#pragma once

#include "dxgi_texture.h"

class DXGITextureStaging : public DXGITexture {
public:
    DXGITextureStaging(Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context,
                       Microsoft::WRL::ComPtr<ID3D11Device> d3d_device);
    ~DXGITextureStaging();

    bool CopyFromTexture(const DXGI_OUTDUPL_FRAME_INFO& frame_info,
                         Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture) override;

    bool Release() override;

private:
    void UpdateTextureIfNeed(Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture);

private:
    Microsoft::WRL::ComPtr<ID3D11Texture2D> cpu_access_texture_{};
    Microsoft::WRL::ComPtr<ID3D11Device> d3d_device_{};
    Microsoft::WRL::ComPtr<IDXGISurface> dxgi_surface_{};
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context_{};
};