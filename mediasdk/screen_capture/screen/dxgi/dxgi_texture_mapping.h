#pragma once

#include "dxgi_texture.h"

class DXGITextureMapping : public DXGITexture {
public:
    DXGITextureMapping(Microsoft::WRL::ComPtr<IDXGIOutputDuplication> dxgi_duplication);
    ~DXGITextureMapping();

    bool CopyFromTexture(const DXGI_OUTDUPL_FRAME_INFO& frame_info,
                         Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture) override;

    bool Release() override;

private:
    Microsoft::WRL::ComPtr<IDXGIOutputDuplication> dxgi_duplication_{};
};