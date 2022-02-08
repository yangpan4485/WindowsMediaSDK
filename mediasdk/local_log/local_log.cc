#include "local_log.h"

#include <iostream>

#include "log_env.h"
#include "log_writer.h"

LocalLog::LocalLog(LocalLogLevel log_level, const std::string& log_tag,
                   const std::string& log_filename, uint32_t log_line_num)
    : log_level_(log_level), log_tag_(log_tag), log_filename_(log_filename),
      log_line_num_(log_line_num) {}

LocalLog::~LocalLog() {
    this->Write();
}

void LocalLog::Write() {
    if (log_level_ < LogEnv::CreateInstance()->GetLogLevel()) {
        return;
    }
    auto pos = log_filename_.rfind("\\");
    log_filename_ = log_filename_.substr(pos + 1, log_filename_.length() - pos - 1);
    LogWriter::CreateInstance()->WriteLog(log_level_, log_tag_, log_filename_, log_line_num_,
                                          oss_.str());
}

void SetLocalLogLevel(LocalLogLevel log_level) {
    LogEnv::CreateInstance()->SetLogLevel(log_level);
}

void SetLocalLogDir(const std::string& log_dir) {
    LogEnv::CreateInstance()->SetLogDir(log_dir);
}