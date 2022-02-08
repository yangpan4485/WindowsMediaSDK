#include "bitmap_gdi.h"

BitmapGDI::BitmapGDI(HDC hdc, uint32_t width, uint32_t height,
                     BitmapImageBufferPoolType& bitmap_pool)
    : bitmap_pool_(bitmap_pool) {
    width_ = width;
    height_ = height;

    size_ = width * height * 4;
    bool ret = bitmap_pool_.try_dequeue(bmp_);
    if (ret) {
        BITMAP bm;
        GetObject(bmp_.get(), sizeof(BITMAP), &bm);
        data_ = static_cast<BYTE*>(bm.bmBits);
        if (bm.bmWidth != width || bm.bmHeight != height) {
            ret = false;
        }
    }
    if (!ret) {
        BITMAPINFO bi = { 0 };
        BITMAPINFOHEADER* bih = &bi.bmiHeader;
        bih->biSize = sizeof(BITMAPINFOHEADER);
        bih->biBitCount = 32;
        bih->biWidth = width;
        bih->biHeight = 0 - height;
        bih->biPlanes = 1;

        HBITMAP bitmap = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, (void**)&data_, NULL, 0);
        bmp_.reset(bitmap, [](HBITMAP bm) { DeleteObject(bm); });
    }
}

BitmapGDI::~BitmapGDI() {
    bitmap_pool_.enqueue(bmp_);
    
}

HBITMAP BitmapGDI::GetBitmap() {
    return bmp_.get();
}

uint8_t* BitmapGDI::GetARGBData() {
    return data_;
}

uint32_t BitmapGDI::GetSize() {
    return size_;
}