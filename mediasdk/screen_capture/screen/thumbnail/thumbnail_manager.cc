#include "thumbnail_manager.h"

#include <iostream>
#include <vector>

#include "video_frame.h"
#include "thumbnail_window.h"

#pragma comment(lib, "Gdiplus.lib")

ThumbnailManager::ThumbnailManager() {
    Gdiplus::GdiplusStartupInput input;
    Gdiplus::GdiplusStartupOutput output;
    Gdiplus::Status status = GdiplusStartup(&token_, &input, &output);
}

ThumbnailManager::~ThumbnailManager() {
    Gdiplus::GdiplusShutdown(token_);
}

void ThumbnailManager::SetScreenList(std::vector<ScreenInfo> screen_list) {
    screen_list_ = screen_list;
    // 找到屏幕左上角的位置
    for (size_t i = 0; i < screen_list_.size(); ++i) {
        if (screen_list_[i].screen_rect.left < desktop_x_) {
            desktop_x_ = screen_list_[i].screen_rect.left;
            desktop_y_ = screen_list_[i].screen_rect.right;
        } else if (screen_list_[i].screen_rect.left == desktop_x_) {
            if (screen_list_[i].screen_rect.top < desktop_y_) {
                desktop_y_ = screen_list_[i].screen_rect.top;
            }
        }
    }
    std::cout << "desktop_x_: " << desktop_x_ << std::endl;
    std::cout << "desktop_y_: " << desktop_y_ << std::endl;
}

std::string ThumbnailManager::GetScreenThumbnail(WindowRect screen_rect, uint32_t thumbnail_width,
                                                 uint32_t thumbnail_height) {
    HDC hdc = GetDC(NULL);
    HDC mem_hdc = CreateCompatibleDC(hdc);
    int width = screen_rect.right - screen_rect.left;
    int height = screen_rect.bottom - screen_rect.top;
    HBITMAP bitmap = CreateCompatibleBitmap(hdc, width, height);
    SelectObject(mem_hdc, bitmap);
    BitBlt(mem_hdc, 0, 0, width, height, hdc, screen_rect.left, screen_rect.top, SRCCOPY);
    Gdiplus::Bitmap bitmap_src(bitmap, NULL);
    std::string thumbnail_image =
        GetThumbnailImage(std::move(bitmap_src), thumbnail_width, thumbnail_height);
    DeleteObject(bitmap);
    DeleteDC(mem_hdc);
    ReleaseDC(NULL, hdc);
    return thumbnail_image;
}

std::string ThumbnailManager::GetWindowThumbnail(HWND window_id, uint32_t thumbnail_width,
                                                 uint32_t thumbnail_height) {
    RECT thumbnail_window_rect;
#if 1
    thumbnail_window_rect.left = desktop_x_ - thumbnail_width;
    thumbnail_window_rect.top = desktop_y_ - thumbnail_height;
    thumbnail_window_rect.right = desktop_x_;
    thumbnail_window_rect.bottom = desktop_y_;
#else
    thumbnail_window_rect.left = 0;
    thumbnail_window_rect.top = 0;
    thumbnail_window_rect.right = thumbnail_width;
    thumbnail_window_rect.bottom = thumbnail_height;
#endif
    // ThumbnailWindow 只创建一次，会遇到后面几个窗口的缩略图是空的
    // ScreenCaptureMag 只创建一次，会遇到中间有几个画面的采集是相同的
    ThumbnailWindow thumbnail_window;
    ScreenCaptureMag screen_capture_mag;
    thumbnail_window.Create(thumbnail_window_rect);
    thumbnail_window.SetTarget(window_id);
    std::shared_ptr<VideoFrame> frame;
    for (int i = 0; i < 5; ++i) {
        frame = screen_capture_mag.GetThumbnailWindowImage(thumbnail_window_rect);
        if (frame) {
            break;
        }
    }
    thumbnail_window.Destroy();
    if (!frame) {
        return "";
    }
    Gdiplus::Bitmap bitmap(frame->GetWidth(), frame->GetHeight(), frame->GetWidth() * 4,
                           PixelFormat32bppARGB, frame->GetData());
    std::string thumbnail_image =
        GetThumbnailImage(std::move(bitmap), thumbnail_width, thumbnail_height);
    return thumbnail_image;
}

std::string ThumbnailManager::GetThumbnailImage(Gdiplus::Bitmap&& bitmap_src,
                                                uint32_t thumbnail_width,
                                                uint32_t thumbnail_height) {
    Gdiplus::Bitmap bitmap_dst(thumbnail_width, thumbnail_height);
    Gdiplus::Graphics graphics(&bitmap_dst);
    graphics.Clear(Gdiplus::Color(0, 255, 255, 255));
    if (((float)bitmap_src.GetWidth() / (float)bitmap_src.GetHeight()) >=
        ((float)bitmap_dst.GetWidth() / (float)bitmap_dst.GetHeight())) {
        float scale_x = (float)bitmap_dst.GetWidth() / (float)bitmap_src.GetWidth();
        int diff_height = (int)(bitmap_dst.GetHeight() - scale_x * bitmap_src.GetHeight());
        graphics.DrawImage(&bitmap_src, 0, diff_height / 2, bitmap_dst.GetWidth(),
                           bitmap_dst.GetHeight() - diff_height);
    } else {
        float scale_y = (float)bitmap_dst.GetHeight() / (float)bitmap_src.GetHeight();
        int diff_width = (int)(bitmap_dst.GetWidth() - scale_y * bitmap_src.GetWidth());
        graphics.DrawImage(&bitmap_src, diff_width / 2, 0, bitmap_dst.GetWidth() - diff_width,
                           bitmap_dst.GetHeight());
    }
    CLSID clsid;
    CLSIDFromString(L"{557cf406-1a04-11d3-9a73-0000f81ef32e}", &clsid);
    IStream* istream = NULL;
    CreateStreamOnHGlobal(NULL, TRUE, &istream);
    bitmap_dst.Save(istream, &clsid);
    HGLOBAL hg = NULL;
    GetHGlobalFromStream(istream, &hg);
    size_t size = GlobalSize(hg);
    std::vector<char> image_data;
    image_data.resize(size);
    LPVOID buffer = GlobalLock(hg);
    memcpy(&image_data[0], buffer, size);
    GlobalUnlock(hg);
    return std::string(image_data.begin(), image_data.end());
}