#include "log_utils.h"
#include <algorithm>
#include <atlstr.h>
#include <io.h>
#include <iomanip>
#include <shlobj.h>
#include <sstream>
#include <strsafe.h>
#include <tlhelp32.h>
#include <windows.h>
#pragma comment(lib, "User32.lib")

std::string GetPlatfromDefaultDir() {
    CHAR szPath[MAX_PATH];
    SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, szPath);
    std::string root_path = szPath;
    root_path += "\\MediaSDK\\LocalLog\\";
    return root_path;
}

std::string GenLogFilename() {
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    std::stringstream ss;
    ss << wtm.wYear << "-" << std::setw(2) << std::setfill('0') << wtm.wMonth << "-" << std::setw(2)
       << std::setfill('0') << wtm.wDay << "_" << std::setw(2) << std::setfill('0') << wtm.wHour
       << std::setw(2) << std::setfill('0') << wtm.wMinute << std::setw(2) << std::setfill('0')
       << wtm.wSecond;
    return ss.str();
}

std::string GetCurrentTimeStr() {
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << wtm.wHour << ":" << std::setw(2) << std::setfill('0')
       << wtm.wMinute << ":" << std::setw(2) << std::setfill('0') << wtm.wSecond << "."
       << std::setw(3) << std::setfill('0') << wtm.wMilliseconds;
    return ss.str();
}

bool IsDirExist(const std::string& dirname) {
    struct _stat dir_stat {};
    if (_stat(dirname.c_str(), &dir_stat) == 0 && dir_stat.st_mode & _S_IFDIR) {
        return true;
    }
    return false;
}

bool Mkdirs(const std::string& dirname) {
    const char* strDirPath = dirname.c_str();
    int ipathLength = strlen(strDirPath);
    int ileaveLength = 0;
    int iCreatedLength = 0;
    char szPathTemp[MAX_PATH] = {0};
    bool result = false;
    for (int i = 0; (NULL != strchr(strDirPath + iCreatedLength, '\\')); i++) {
        ileaveLength = strlen(strchr(strDirPath + iCreatedLength, '\\')) - 1;
        iCreatedLength = ipathLength - ileaveLength;
        strncpy(szPathTemp, strDirPath, iCreatedLength);
        result = Mkdir(szPathTemp);
    }
    if (iCreatedLength < ipathLength) {
        result = Mkdir(strDirPath);
    }
    return result;
}

bool Mkdir(const std::string& dirname) {
    if (IsDirExist(dirname)) {
        return true;
    }
    if (CreateDirectoryA(dirname.c_str(), NULL)) {
        return true;
    }
    return GetLastError() == ERROR_ALREADY_EXISTS;
}

bool RemoveFile(const std::string& filename) {
    int result = std::remove(filename.c_str());
    return result == 0;
}

bool IsDigits(const std::string& str) {
    return std::all_of(str.begin(), str.end(), ::isdigit);
}

DWORD PlatformThreadId() {
    return GetCurrentThreadId();
}

DWORD GetCurrentFileSize(const std::string& filename) {
    struct stat info;
    if (stat(filename.c_str(), &info) != 0) {
        return 0;
    }
    return info.st_size;
}

std::vector<std::string> GetDirFiles(const std::string& path) {
    std::vector<std::string> files_name;
    WIN32_FIND_DATAA ffd;
    HANDLE handle = FindFirstFileA((path + "\\*").c_str(), &ffd);
    if (INVALID_HANDLE_VALUE != handle) {
        do {
            if (ffd.cFileName[0] == '.')
                continue;
            if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                continue;
            }
            files_name.push_back(ffd.cFileName);
        } while (FindNextFileA(handle, &ffd) != 0);
    }
    FindClose(handle);
    return files_name;
}