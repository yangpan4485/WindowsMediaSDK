#pragma once
#include <cstdint>
#include <fstream>
#include <mutex>
#include <string>
#include <unordered_map>

#include "log_common.h"

class LogWriter {
public:
    static LogWriter* CreateInstance();

    LogWriter();
    ~LogWriter();
    LogWriter(const LogWriter&) = default;
    LogWriter& operator=(const LogWriter&) = default;

    void WriteLog(LocalLogLevel log_level, const std::string& log_tag,
                  const std::string& log_filename, uint32_t log_line_num,
                  const std::string& log_content);
    void SaveFile();
    void SetLocalLogSetting(uint32_t expired_time, uint64_t max_log_size);

private:
    void NewFile(const std::string& filename);
    void HandleExpiredLog();
    void HandOversizeLog();

private:
    std::unordered_map<LocalLogLevel, std::string> log_map_{};
    std::ofstream fout_{};
    std::mutex mtx_{};
    uint32_t expired_time_{};
    uint64_t max_log_size_{};
    std::string dirname_{};
};