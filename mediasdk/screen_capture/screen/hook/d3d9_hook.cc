#include "d3d9_hook.h"

#include <iostream>

#include "detours.h"

const int kStretchRectIndex = 34;

D3D9Hook& D3D9Hook::GetInstance() {
    static D3D9Hook instance;
    return instance;
}

bool D3D9Hook::HookStretchRect(HookStretchRectFunc* stretch_rect_func) {
    InitStretchRectHookFunc();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((void**)&stretch_hook_func_, stretch_rect_func);
    DetourTransactionCommit();
    return true;
}

void D3D9Hook::UnhookStretchRect(HookStretchRectFunc* stretch_rect_func) {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach((void**)&stretch_hook_func_, stretch_rect_func);
    DetourTransactionCommit();
    stretch_hook_func_ = nullptr;
}

HookStretchRectFunc* D3D9Hook::GetStretchRectHookFunc() {
    return stretch_hook_func_;
}

D3D9Hook::D3D9Hook() {}

D3D9Hook::~D3D9Hook() {}

void D3D9Hook::InitStretchRectHookFunc() {
    IDirect3D9Ex* d3d9 = nullptr;
    HRESULT hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &d3d9);
    if (FAILED(hr) || !d3d9) {
        return;
    }
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = true;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    IDirect3DDevice9Ex* d3d9_device = nullptr;
    hr = d3d9->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, GetDesktopWindow(),
                              D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, nullptr, &d3d9_device);
    if (FAILED(hr) || !d3d9_device) {
        d3d9->Release();
        return;
    }
    void** vtable_func = *(void***)d3d9_device;
    stretch_hook_func_ = (HookStretchRectFunc*)(vtable_func[kStretchRectIndex]);
    d3d9_device->Release();
    d3d9->Release();
}