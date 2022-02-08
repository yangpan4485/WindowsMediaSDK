#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <memory>
#include <wrl/client.h>

class DXGITexture {
public:
    DXGITexture();
    virtual ~DXGITexture();

    virtual bool CopyFromTexture(const DXGI_OUTDUPL_FRAME_INFO& frame_info,
                                 Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture);

    virtual bool Release();
    virtual uint8_t* GetARGBData();
    virtual uint32_t GetPitch();

protected:
    DXGI_MAPPED_RECT dxgi_rect_{};
};