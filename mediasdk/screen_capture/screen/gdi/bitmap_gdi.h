#pragma once
#include <windows.h>
#include <memory>

#include "common/concurrentqueue.h"

using BitmapImageBufferPoolType = moodycamel::ConcurrentQueue<std::shared_ptr<HBITMAP__>>;

class BitmapGDI {
public:
    BitmapGDI(HDC hdc, uint32_t width, uint32_t height, BitmapImageBufferPoolType& bitmap_pool);
    ~BitmapGDI();
    HBITMAP GetBitmap();
    uint8_t* GetARGBData();
    uint32_t GetSize();

private:
    BitmapImageBufferPoolType& bitmap_pool_;
    std::shared_ptr<HBITMAP__> bmp_;
    uint32_t size_{};
    uint32_t width_{};
    uint32_t height_{};
    uint8_t* data_{};
};