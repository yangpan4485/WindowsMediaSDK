#include "screen_capture_dxgi.h"

#include <iostream>

#include "dxgi_texture.h"
#include "dxgi_texture_mapping.h"
#include "dxgi_texture_staging.h"
#include "screen/window_utils.h"
#include "video_frame.h"

#define kBytesPerPixel 4

ScreenCaptureDX::ScreenCaptureDX() {}

ScreenCaptureDX::~ScreenCaptureDX() {
    Uninit();
}

std::shared_ptr<VideoFrame> ScreenCaptureDX::CaptureScreen(const ScreenInfo& screen_info) {
    if (!init_) {
        Init();
        // Init(screen_info.display_id);
        init_ = true;
    }
    return GetFrame();
}

bool ScreenCaptureDX::Uninit() {
    d3d_device_.Reset();
    context_.Reset();
    dxgi_device_.Reset();
    output_.Reset();
    duplication_.Reset();
    return true;
}

bool ScreenCaptureDX::Init() {
    std::vector<ComPtr<IDXGIAdapter>> adapters = EnumAdapters();
    HRESULT hr = S_OK;
    for (size_t i = 0; i < adapters.size(); ++i) {
        D3D_FEATURE_LEVEL feature_level;
        hr = D3D11CreateDevice(adapters[i].Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL,
                               D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, D3D11_SDK_VERSION,
                               d3d_device_.GetAddressOf(), &feature_level, context_.GetAddressOf());
        if (!InitializeDXGI()) {
            continue;
        }

        ComPtr<IDXGIOutput> dxgi_output;
        UINT output = 0;
        bool is_found = false;
        for (; (hr = adapters[i]->EnumOutputs(output, dxgi_output.ReleaseAndGetAddressOf())) !=
               DXGI_ERROR_NOT_FOUND;
             ++output) {
            DXGI_OUTPUT_DESC desc{};
            hr = dxgi_output->GetDesc(&desc);
            if (!desc.AttachedToDesktop) {
                continue;
            }
            if (desc.Monitor != screen_info_.monitor) {
                continue;
            }
            hr = dxgi_output.As(&output_);
            output_->DuplicateOutput(d3d_device_.Get(), duplication_.GetAddressOf());
            is_found = true;
            break;
        }
        if (is_found) {
            break;
        }
    }
    dxgi_texture_.reset(new DXGITextureStaging(context_, d3d_device_));
    DXGI_OUTDUPL_DESC outdupl_desc;
    memset(&outdupl_desc, 0, sizeof(outdupl_desc));
    duplication_->GetDesc(&outdupl_desc);
    if (outdupl_desc.ModeDesc.Format != DXGI_FORMAT_B8G8R8A8_UNORM) {
        dxgi_texture_.reset(new DXGITextureMapping(duplication_));
    } else {
        dxgi_texture_.reset(new DXGITextureStaging(context_, d3d_device_));
    }

    return true;
}

std::vector<ComPtr<IDXGIAdapter>> ScreenCaptureDX::EnumAdapters() {
    ComPtr<IDXGIFactory1> factory;
    std::vector<ComPtr<IDXGIAdapter>> adapter_vec;
    HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1),
                                    reinterpret_cast<void**>(factory.GetAddressOf()));
    for (int i = 0;; i++) {
        ComPtr<IDXGIAdapter> adapter;
        hr = factory->EnumAdapters(i, adapter.GetAddressOf());
        if (hr == DXGI_ERROR_NOT_FOUND) {
            break;
        }
        if (hr != S_OK) {
            continue;
        }
        adapter_vec.push_back(adapter);
    }
    return adapter_vec;
}

bool ScreenCaptureDX::Init(int display_id) {
    HRESULT hr;
    D3D_DRIVER_TYPE DriverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT NumDriverTypes = ARRAYSIZE(DriverTypes);
    // Feature levels supported
    D3D_FEATURE_LEVEL FeatureLevels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
                                         D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_1};
    UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);
    D3D_FEATURE_LEVEL FeatureLevel;

    for (UINT index = 0; index < NumDriverTypes; index++) {
        d3d_device_.Reset();
        context_.Reset();
        hr = D3D11CreateDevice(NULL, DriverTypes[index], NULL, 0, FeatureLevels, NumFeatureLevels,
                               D3D11_SDK_VERSION, d3d_device_.GetAddressOf(), &FeatureLevel,
                               context_.GetAddressOf());

        if (SUCCEEDED(hr)) {
            if (InitializeDXGI() && SetDisplayId(display_id)) {
                return true;
            }
        }
    }
    return false;
}

bool ScreenCaptureDX::Release() {
    return true;
}

bool ScreenCaptureDX::InitializeDXGI() {
    HRESULT hr;
    dxgi_device_.Reset();
    hr = d3d_device_.As(&dxgi_device_);
    if (FAILED(hr)) {
        return false;
    }
    return true;
}

std::shared_ptr<VideoFrame> ScreenCaptureDX::GetFrame() {
    HRESULT hr;
    if (!duplication_) {
        hr = output_->DuplicateOutput(d3d_device_.Get(), duplication_.GetAddressOf());
        if (FAILED(hr)) {
            return nullptr;
        }
    }

    DXGI_OUTDUPL_FRAME_INFO frame_info = {0};
    Microsoft::WRL::ComPtr<IDXGIResource> resource;
    hr = duplication_->AcquireNextFrame(100, &frame_info, resource.GetAddressOf());

    switch (hr) {
    case S_OK: {
        break;
    }
    case DXGI_ERROR_ACCESS_LOST: {
        duplication_->ReleaseFrame();
        duplication_.Reset();
        WindowRect screen_rect = GetScreenRect(screen_info_.display_id);
        if (!IsEqualRect(screen_rect, screen_info_.screen_rect)) {
            screen_info_.screen_rect = screen_rect;
            Uninit();
            Init(screen_info_.display_id);
        }
        return nullptr;
    }
    case DXGI_ERROR_INVALID_CALL: {
        duplication_.Reset();
        return nullptr;
    }
    case DXGI_ERROR_DEVICE_REMOVED: {
        return nullptr;
    }
    case DXGI_ERROR_WAIT_TIMEOUT: {
        return nullptr;
    }
    default: {
        return nullptr;
    }
    }

    Microsoft::WRL::ComPtr<ID3D11Texture2D> gpu_texture;
    D3D11_TEXTURE2D_DESC desc;
    resource.As(&gpu_texture);
    gpu_texture->GetDesc(&desc);
    std::shared_ptr<VideoFrame> video_frame(
        new VideoFrame(desc.Width, desc.Height, kFrameTypeARGB, true));

    dxgi_texture_->CopyFromTexture(frame_info, gpu_texture);
    uint8_t* src = dxgi_texture_->GetARGBData();
    uint32_t src_pitch = dxgi_texture_->GetPitch();
    uint8_t* dst = video_frame->GetData();
    uint32_t dst_pitch = video_frame->GetPitch();
    for (int i = 0; i < desc.Height; i++) {
        memcpy(dst + i * dst_pitch, src + i * src_pitch, desc.Width * 4);
    }
    dxgi_texture_->Release();

    if (frame_info.LastMouseUpdateTime.QuadPart != 0) {
        current_pointer_positon_ = frame_info.PointerPosition;
    }
    if (frame_info.PointerShapeBufferSize > 0) {
        if (frame_info.PointerShapeBufferSize > cursor_buffer_.size()) {
            cursor_buffer_.resize(frame_info.PointerShapeBufferSize);
        }
        UINT require;
        hr = duplication_->GetFramePointerShape(frame_info.PointerShapeBufferSize,
                                                &cursor_buffer_[0], &require, &pointer_info_);
    }
    if (enable_cursor_ && current_pointer_positon_.Visible) {
        draw_cursor_.Draw(video_frame, current_pointer_positon_, pointer_info_, &cursor_buffer_[0]);
    }
    duplication_->ReleaseFrame();
    return video_frame;
}

bool ScreenCaptureDX::SetDisplayId(uint32_t display_id) {
    HRESULT hr;
    Microsoft::WRL::ComPtr<IDXGIOutput> output;
    Microsoft::WRL::ComPtr<IDXGIAdapter> dxgi_adapter;

    duplication_.Reset();
    output_.Reset();

    hr = dxgi_device_->GetAdapter(&dxgi_adapter);
    if (FAILED(hr)) {
        return false;
    }
    hr = dxgi_adapter->EnumOutputs(display_id, output.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    hr = output.As(&output_);
    if (FAILED(hr)) {
        return false;
    }

    // Create desktop duplication
    hr = output_->DuplicateOutput(d3d_device_.Get(), duplication_.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }
    dxgi_texture_.reset(new DXGITextureStaging(context_, d3d_device_));
    return true;
}