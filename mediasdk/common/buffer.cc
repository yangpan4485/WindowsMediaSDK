#include "buffer.h"

Buffer::Buffer(uint32_t size) : size_(size) {
    data_ = new uint8_t[size_];
}

Buffer::~Buffer() {
    if (data_) {
        delete data_;
        data_ = nullptr;
    }
}

uint32_t Buffer::GetSize() {
    return size_;
}

uint8_t* Buffer::GetData() {
    return data_;
}