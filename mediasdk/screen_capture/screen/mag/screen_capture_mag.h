#pragma once
#include <Windows.h>
#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>
#include <d3d9.h>
#include <magnification.h>

#include "video_frame.h"
#include "screen/screen_capture.h"

class ScreenCaptureMag : public ScreenCapture {
public:
    ScreenCaptureMag();
    ~ScreenCaptureMag();

    std::shared_ptr<VideoFrame> CaptureWindow(const WindowInfo& window_info) override;
    std::shared_ptr<VideoFrame> CaptureScreen(const ScreenInfo& screen_info) override;
    std::shared_ptr<VideoFrame> GetThumbnailWindowImage(RECT thumbnail_window_rect);

public:
    // void SetCaptureRECT(HWND hwnd, const RECT& rect);
    bool Init();
    bool Uninit();
    std::shared_ptr<VideoFrame> CaptureFrame(const RECT& rect, bool capture_cursor);
    // 如果有分辨率限制的话需要设置 scale size
    // void UpdateScaleSize(int width, int height);
    // 窗口采集时，当窗口位置变化了，需要更新窗口位置
    void UpdateMagWindowPos(const RECT& rect);
    bool CreateMagWindow();
    bool DestroyMagWindow();

private:
    void OnMagCallback(int width, int height, uint8_t* data);
    void UpdateCursor();
    void SetFilterWindow(const std::vector<HWND>& filter_window);
    void OnStretchRect(IDirect3DDevice9Ex* d3d9_device, IDirect3DSurface9* d3d9_surface);
    void CalcLastFrameSize(int& width, int& height);
    bool CheckSizeUpdate();
    static BOOL CALLBACK OnMagImageScalingCallback(HWND hwnd, void* srcdata,
                                                   MAGIMAGEHEADER srcheader, void* destdata,
                                                   MAGIMAGEHEADER destheader, RECT unclipped,
                                                   RECT clipped, HRGN dirty);
    static LRESULT CALLBACK HostWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static HRESULT CALLBACK StretchRectHook(IDirect3DDevice9Ex* d3d_device,
                                            IDirect3DSurface9* d3d_src_surface,
                                            const RECT* src_rect,
                                            IDirect3DSurface9* d3d_dest_surface,
                                            const RECT* dest_rect, D3DTEXTUREFILTERTYPE filter);

private:
    HWND capture_hwnd_{};
    RECT capture_rect_{};
    bool enable_d3d9_hook_{};
    bool enable_mag_callback_{};
    bool capture_result_{};
    bool enable_cursor_{};
    bool init_{};
    bool capture_thumbnail_{};
    int capture_width_{};
    int capture_height_{};
    int frame_width_{};
    int frame_height_{};
    std::shared_ptr<VideoFrame> capture_frame_{};
    IDirect3DSurface9* render_surface_{};
    IDirect3DSurface9* off_screen_surface_{};
    RECT mag_window_rect_{};
    RECT last_thumbnail_rect_{};

    HWND host_window_{};
    HWND control_window_{};

    std::vector<HWND> over_window_{};
    ScreenInfo mag_screen_info_{};
    std::unordered_map<HWND, bool> over_window_map_{};
};
