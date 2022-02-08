#include "dxgi_texture_mapping.h"

DXGITextureMapping::DXGITextureMapping(
    Microsoft::WRL::ComPtr<IDXGIOutputDuplication> dxgi_duplication)
    : dxgi_duplication_(dxgi_duplication) {}

DXGITextureMapping::~DXGITextureMapping() {
}

bool DXGITextureMapping::CopyFromTexture(const DXGI_OUTDUPL_FRAME_INFO& frame_info,
                                         Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture) {
    dxgi_duplication_->MapDesktopSurface(&dxgi_rect_);
    return true;
}

bool DXGITextureMapping::Release() {
    dxgi_duplication_->UnMapDesktopSurface();
    return true;
}