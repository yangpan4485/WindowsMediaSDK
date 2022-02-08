#include "task_thread.h"

#include <iostream>

TaskThread::TaskThread() {
    running_ = true;
    work_thread_ = std::thread([this]() {
        while (running_) {
            TaskWork task;
            {
                std::unique_lock<std::mutex> lock(mtx_);
                if (work_queue_.empty()) {
                    con_.wait(lock, [&]() {
                        return !running_ || !work_queue_.empty();
                    });
                }
                if (!running_) {
                    break;
                }
                task = work_queue_.front();
                work_queue_.pop();
            }
            task();
        }
    });
}

TaskThread::~TaskThread() {
    Wait();
}

void TaskThread::PostWork(TaskWork task_work) {
    {
        std::unique_lock<std::mutex> lock(mtx_);
        work_queue_.push(task_work);
    }
    con_.notify_all();
}

void TaskThread::Wait() {
    running_ = false;
    con_.notify_all();
    if (work_thread_.joinable()) {
        work_thread_.join();
    }
    std::cout << "close: " << work_queue_.size() << std::endl;
}