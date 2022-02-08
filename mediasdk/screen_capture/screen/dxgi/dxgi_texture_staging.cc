#include "dxgi_texture_staging.h"

#include <iostream>

DXGITextureStaging::DXGITextureStaging(Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context,
                                       Microsoft::WRL::ComPtr<ID3D11Device> d3d_device)
    : device_context_(device_context), d3d_device_(d3d_device) {}

DXGITextureStaging::~DXGITextureStaging() {}

bool DXGITextureStaging::CopyFromTexture(const DXGI_OUTDUPL_FRAME_INFO& frame_info,
                                         Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture) {
    UpdateTextureIfNeed(texture);
    device_context_->CopyResource(static_cast<ID3D11Resource*>(cpu_access_texture_.Get()),
                                  static_cast<ID3D11Resource*>(texture.Get()));
    HRESULT hr = dxgi_surface_->Map(&dxgi_rect_, DXGI_MAP_READ);
    if (FAILED(hr)) {
        std::cout << "CopyFromTexture end hr : " << hr << ", :" << DXGI_ERROR_DEVICE_REMOVED << std::endl;
        return false;
    }
    return true;
}

void DXGITextureStaging::UpdateTextureIfNeed(Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture) {
    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);
    D3D11_TEXTURE2D_DESC current_desc;
    if (cpu_access_texture_) {
        cpu_access_texture_->GetDesc(&current_desc);
        if (memcmp(&desc, &current_desc, sizeof(D3D11_TEXTURE2D_DESC)) == 0) {
            return;
        }
        cpu_access_texture_.Reset();
        dxgi_surface_.Reset();
    }
    current_desc.Width = desc.Width;
    current_desc.Height = desc.Height;
    current_desc.ArraySize = 1;
    current_desc.MipLevels = 1;
    current_desc.BindFlags = 0;
    current_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    current_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    current_desc.SampleDesc.Count = 1;
    current_desc.MiscFlags = 0;
    current_desc.SampleDesc.Count = 1;
    current_desc.SampleDesc.Quality = 0;
    current_desc.Usage = D3D11_USAGE_STAGING;
    d3d_device_->CreateTexture2D(&current_desc, nullptr, cpu_access_texture_.GetAddressOf());
    cpu_access_texture_.As(&dxgi_surface_);
}

// 使用完数据后需要调用 Release()
bool DXGITextureStaging::Release() {
    dxgi_surface_->Unmap();
    return true;
}
