#pragma once
#include "screen_event_handler.h"
#include <fstream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

class MainWindow;

class EventCallback : public ICaptureHandler {
public:
    EventCallback();
    ~EventCallback();

    void OnScreenFrame(std::shared_ptr<VideoFrame> video_frame) override;
    void OnScreenEvent(ScreenEvent screen_event) override;

    void RegisteObserver(MainWindow* main_window);

private:
    std::ofstream fout_{};
    uint8_t* y_data_{};
    uint8_t* u_data_{};
    uint8_t* v_data_{};
    int width_;
    int height_;
    std::thread work_thread_{};
    std::mutex mtx_{};
    std::condition_variable cv_{};
    std::queue<std::shared_ptr<VideoFrame>> frame_queue_{};
    bool running_{};
    MainWindow* main_window_{};
};