#include "log_writer.h"

#include <algorithm>
#include <iostream>
#include <sstream>

#include "log_env.h"
#include "log_utils.h"

LogWriter* LogWriter::CreateInstance() {
    static LogWriter* instance = new LogWriter();
    return instance;
}

LogWriter::LogWriter() {
    log_map_[kLocalLogLevelDebug] = "D";
    log_map_[kLocalLogLevelInfo] = "I";
    log_map_[kLocalLogLevelWarning] = "W";
    log_map_[kLocalLogLevelError] = "E";
    log_map_[kLocalLogLevelNone] = "N";
}

LogWriter::~LogWriter() {
    if (fout_.is_open()) {
        fout_.close();
    }
    HandleExpiredLog();
}

void LogWriter::WriteLog(LocalLogLevel log_level, const std::string& log_tag,
                         const std::string& log_filename, uint32_t log_line_num,
                         const std::string& log_content) {
    std::unique_lock<std::mutex> lock(mtx_);
    if (dirname_.empty()) {
        dirname_ = LogEnv::CreateInstance()->GetLogDir() + SEPARATOR + "Logs";
        if (!IsDirExist(dirname_)) {
            Mkdirs(dirname_);
        }
    }
    if (!fout_ || !fout_.is_open()) {
        std::string filename = dirname_ + SEPARATOR + GenLogFilename() + ".log";
        NewFile(filename);
    }
    /*size_t filesize = (size_t)fout_.tellp();
    if (filesize + log_content.length() > LogEnv::CreateInstance()->GetSingleLogFileSize()) {
        std::string filename = dirname_ + SEPARATOR + GenLogFilename() + ".log";
        NewFile(filename);
    }*/

    std::stringstream ss;
    fout_ << GetCurrentTimeStr() << "[" << log_tag << "][" << log_map_[log_level] << "]["
          << PlatformThreadId() << "](" << log_filename << ":" << log_line_num << "): ";
    fout_ << log_content;
    if (log_content[log_content.length() - 1] != '\n') {
        fout_ << "\n";
    }
}

void LogWriter::SaveFile() {
    std::unique_lock<std::mutex> lock(mtx_);
    if (fout_.is_open()) {
        fout_.close();
    }
}

void LogWriter::SetLocalLogSetting(uint32_t expired_time, uint64_t max_log_size) {
    std::unique_lock<std::mutex> lock(mtx_);
    expired_time_ = expired_time;
    max_log_size_ = max_log_size;
}

void LogWriter::NewFile(const std::string& filename) {
    HandOversizeLog();
    if (fout_.is_open()) {
        fout_.close();
    }
    fout_.open(filename, std::ios::binary | std::ios::out);
}

void LogWriter::HandleExpiredLog() {}

void LogWriter::HandOversizeLog() {
    if (dirname_.empty() || !IsDirExist(dirname_)) {
        return;
    }
    std::vector<std::string> files = GetDirFiles(dirname_);
    std::sort(files.begin(), files.end(),
              [](const std::string& str1, const std::string& str2) { return str1 > str2; });
    size_t i = 0;
    uint32_t total_filesize = 0;
    for (; i < files.size(); ++i) {
        total_filesize += GetCurrentFileSize(dirname_ + SEPARATOR + files[i]);
        if (total_filesize > LogEnv::CreateInstance()->GetMaxLogSize()) {
            break;
        }
    }
    for (; i < files.size(); ++i) {
        RemoveFile(dirname_ + SEPARATOR + files[i]);
    }
}