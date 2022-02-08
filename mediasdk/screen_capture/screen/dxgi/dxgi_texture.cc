#include "dxgi_texture.h"

DXGITexture::DXGITexture() {}

DXGITexture::~DXGITexture() {}

bool DXGITexture::CopyFromTexture(const DXGI_OUTDUPL_FRAME_INFO& frame_info,
                                  Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture) {
    return true;
}

bool DXGITexture::Release() {
    return true;
}

uint8_t* DXGITexture::GetARGBData() {
    return dxgi_rect_.pBits;
}

uint32_t DXGITexture::GetPitch() {
    return dxgi_rect_.Pitch;
}