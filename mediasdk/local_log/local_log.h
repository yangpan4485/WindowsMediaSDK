#pragma once
#include "log_common.h"
#include <sstream>
#include <string>

class LocalLog {
public:
    LocalLog(LocalLogLevel log_level, const std::string& log_tag, const std::string& log_filename,
             uint32_t log_line_num);

    ~LocalLog();

    template <typename T> LocalLog& operator<<(const T& t) {
        oss_ << t;
        return *this;
    }

    // if pointer of char is NULL
    template <typename T> LocalLog& operator<<(T* t) {
        if (t == nullptr) {
            oss_ << "";
        } else {
            oss_ << t;
        }
        return *this;
    }

private:
    void Write();

private:
    LocalLogLevel log_level_{};
    std::string log_tag_{};
    std::string log_filename_{};
    uint32_t log_line_num_{};
    std::ostringstream oss_;
};

#define LOGD(log_tag) LocalLog(kLocalLogLevelDebug, log_tag, __FILE__, __LINE__)
#define LOGI(log_tag) LocalLog(kLocalLogLevelInfo, log_tag, __FILE__, __LINE__)
#define LOGW(log_tag) LocalLog(kLocalLogLevelWarning, log_tag, __FILE__, __LINE__)
#define LOGE(log_tag) LocalLog(kLocalLogLevelError, log_tag, __FILE__, __LINE__)
#define LOGN(log_tag) LocalLog(kLocalLogLevelNone, log_tag, __FILE__, __LINE__)

void SetLocalLogLevel(LocalLogLevel log_level);
void SetLocalLogDir(const std::string& log_dir);