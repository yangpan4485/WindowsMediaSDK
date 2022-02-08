#pragma once
#include <cstdint>
#include <mutex>
#include <string>

#include "log_common.h"

class LogEnv {
public:
    static LogEnv* CreateInstance();

    void SetSingleLogFileSize(uint32_t filesize);
    uint32_t GetSingleLogFileSize();

    void SetMaxLogSize(uint64_t max_log_size);
    uint64_t GetMaxLogSize();

    void SetLogExpiredTime(uint32_t expired_time);
    uint32_t GetLogExpiredTime();

    void SetLogDir(const std::string& log_dir);
    std::string GetLogDir();

    void SetLogLevel(LocalLogLevel log_level_);
    LocalLogLevel GetLogLevel();

private:
    LogEnv();
    ~LogEnv();
    LogEnv(const LogEnv&) = delete;
    LogEnv operator=(const LogEnv&) = delete;

private:
    std::mutex mtx_;
    uint32_t filesize_{};
    uint64_t max_log_size_{};
    uint32_t expired_time_{};
    std::string log_dir_{};
    LocalLogLevel log_level_{kLocalLogLevelInfo};
};