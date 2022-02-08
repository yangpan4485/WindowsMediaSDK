#pragma once
#include <Windows.h>
#include <string>
#include <vector>

#define SEPARATOR "\\"

std::string GetPlatfromDefaultDir();
std::string GenLogFilename();
std::string GetCurrentTimeStr();
bool IsDirExist(const std::string& dirname);
bool Mkdirs(const std::string& dirname);
bool Mkdir(const std::string& dirname);
bool RemoveFile(const std::string& filename);
bool IsDigits(const std::string& str);
DWORD PlatformThreadId();
DWORD GetCurrentFileSize(const std::string& filename);
std::vector<std::string> GetDirFiles(const std::string& path);