#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>

#include "buffer.h"
#include "concurrentqueue.h"
#include "video_frame.h"

using BufferPoolQueue = moodycamel::ConcurrentQueue<std::shared_ptr<Buffer>>;
class BufferPool {
public:
    static BufferPool& GetInstance();
    BufferPoolQueue& GetBufferPool(uint32_t size);

private:
    BufferPool();
    ~BufferPool();
    BufferPool(const BufferPool&) = delete;
    BufferPool operator=(const BufferPool&) = delete;

private:
    std::mutex buffer_pool_mtx_{};
    std::unordered_map<uint32_t, BufferPoolQueue> buffer_pool_map_{};
};