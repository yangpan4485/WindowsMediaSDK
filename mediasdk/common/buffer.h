#pragma once

#include <cstdint>

class Buffer {
public:
    Buffer(uint32_t size);
    ~Buffer();

    uint32_t GetSize();
    uint8_t* GetData();

private:
    uint32_t size_{};
    uint8_t* data_{};
};