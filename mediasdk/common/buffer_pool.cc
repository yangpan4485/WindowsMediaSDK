#include "buffer_pool.h"

BufferPool& BufferPool::GetInstance() {
    static BufferPool instance;
    return instance;
}

BufferPool::BufferPool() {}

BufferPool::~BufferPool() {}

BufferPoolQueue& BufferPool::GetBufferPool(uint32_t size) {
    return buffer_pool_map_[size];
}
