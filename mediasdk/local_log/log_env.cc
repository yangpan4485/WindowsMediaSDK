#include "log_env.h"

#include "log_utils.h"

const uint32_t kDefaultFileSize = 10 * 1024 * 1024;         // 4MB
const uint64_t kDefaultMaxLogSize = 1 * 1024 * 1024 * 1024; // 1GB
const uint32_t kDefaultExpiredTime = 7;                     // 7day

LogEnv* LogEnv::CreateInstance() {
    static LogEnv* instance = new LogEnv();
    return instance;
}

void LogEnv::SetSingleLogFileSize(uint32_t filesize) {
    std::unique_lock<std::mutex> lock(mtx_);
    filesize_ = filesize;
}

uint32_t LogEnv::GetSingleLogFileSize() {
    std::unique_lock<std::mutex> lock(mtx_);
    return filesize_;
}

void LogEnv::SetMaxLogSize(uint64_t max_log_size) {
    std::unique_lock<std::mutex> lock(mtx_);
    max_log_size_ = max_log_size;
}

uint64_t LogEnv::GetMaxLogSize() {
    std::unique_lock<std::mutex> lock(mtx_);
    return max_log_size_;
}

void LogEnv::SetLogExpiredTime(uint32_t expired_time) {
    std::unique_lock<std::mutex> lock(mtx_);
    expired_time_ = expired_time;
}

uint32_t LogEnv::GetLogExpiredTime() {
    std::unique_lock<std::mutex> lock(mtx_);
    return expired_time_;
}

void LogEnv::SetLogDir(const std::string& log_dir) {
    std::unique_lock<std::mutex> lock(mtx_);
    log_dir_ = log_dir;
}

std::string LogEnv::GetLogDir() {
    std::unique_lock<std::mutex> lock(mtx_);
    return log_dir_;
}

void LogEnv::SetLogLevel(LocalLogLevel log_level_) {
    std::unique_lock<std::mutex> lock(mtx_);
    log_level_ = log_level_;
}

LocalLogLevel LogEnv::GetLogLevel() {
    std::unique_lock<std::mutex> lock(mtx_);
    return log_level_;
}

LogEnv::LogEnv() {
    filesize_ = kDefaultFileSize;
    max_log_size_ = kDefaultMaxLogSize;
    expired_time_ = kDefaultExpiredTime;
    log_dir_ = GetPlatfromDefaultDir();
}

LogEnv::~LogEnv() {}