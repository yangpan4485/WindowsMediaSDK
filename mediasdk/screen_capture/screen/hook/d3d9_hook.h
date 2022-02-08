#pragma once
#include <d3d9.h>
#include <windows.h>

typedef HRESULT CALLBACK HookStretchRectFunc(IDirect3DDevice9Ex* d3d_device,
                                             IDirect3DSurface9* d3d_src_surface,
                                             const RECT* src_rect,
                                             IDirect3DSurface9* d3d_dest_surface,
                                             const RECT* dest_rect, D3DTEXTUREFILTERTYPE filter);

class D3D9Hook {
public:
    static D3D9Hook& GetInstance();
    bool HookStretchRect(HookStretchRectFunc* stretch_rect_func);
    void UnhookStretchRect(HookStretchRectFunc* stretch_rect_func);
    HookStretchRectFunc* GetStretchRectHookFunc();

private:
    D3D9Hook();
    ~D3D9Hook();
    D3D9Hook(const D3D9Hook&) = delete;
    D3D9Hook operator=(const D3D9Hook&) = delete;

    void InitStretchRectHookFunc();

private:
    HookStretchRectFunc* stretch_hook_func_{nullptr};
    HWND hwnd_{};
};