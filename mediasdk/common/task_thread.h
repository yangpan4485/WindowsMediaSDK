#pragma once

#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>

using TaskWork = std::function<void()>;
class TaskThread {
public:
    TaskThread();
    ~TaskThread();
    void PostWork(TaskWork task_work);
    void Wait();

private:
    std::mutex mtx_{};
    std::condition_variable con_{};
    std::queue<TaskWork> work_queue_{};
    std::thread work_thread_{};
    bool running_{};
};