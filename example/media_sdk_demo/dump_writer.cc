//#include "dump_writer.h"
//#include <Windows.h>
//#include <direct.h>
//#include <io.h>
//#include <shlobj_core.h>
//#include <tchar.h>
//#include <minidumpapiset.h>
//
//#include <iomanip>
//#include <iostream>
//#include <sstream>
//#include <string>
//
//static std::string kDumpPath = "";
//typedef BOOL(WINAPI* MiniDumpWriteDumpT)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE,
//                                         PMINIDUMP_EXCEPTION_INFORMATION,
//                                         PMINIDUMP_USER_STREAM_INFORMATION,
//                                         PMINIDUMP_CALLBACK_INFORMATION);
//
//int CreateDump(PEXCEPTION_POINTERS pointers) {
//    HMODULE dbg_help = LoadLibrary("DbgHelp.dll");
//    if (NULL == dbg_help) {
//        return EXCEPTION_CONTINUE_EXECUTION;
//    }
//
//    MiniDumpWriteDumpT dump_writer =
//        (MiniDumpWriteDumpT)GetProcAddress(dbg_help, "MiniDumpWriteDump");
//    if (NULL == dump_writer) {
//        FreeLibrary(dbg_help);
//        return EXCEPTION_CONTINUE_EXECUTION;
//    }
//    if (kDumpPath.empty()) {
//        char buffer[256] = "";
//        SHGetSpecialFolderPathA(NULL, buffer, CSIDL_LOCAL_APPDATA, FALSE);
//        kDumpPath = buffer;
//        kDumpPath += "\\MediaSDK";
//    }
//
//    if (_access(kDumpPath.c_str(), 0) == -1) {
//        _mkdir(kDumpPath.c_str());
//    }
//
//    // dmp 文件名
//    SYSTEMTIME local_time;
//    GetLocalTime(&local_time);
//
//    std::stringstream ss;
//    ss << kDumpPath << "\\rtc_meet" << local_time.wYear << "-" << std::setw(2) << std::setfill('0')
//       << local_time.wMonth << "-" << std::setw(2) << std::setfill('0') << local_time.wDay << "-"
//       << std::setw(2) << std::setfill('0') << local_time.wHour << std::setw(2) << std::setfill('0')
//       << local_time.wMinute << std::setw(2) << std::setfill('0') << local_time.wSecond << ".dmp";
//
//    HANDLE dump_file = CreateFile(ss.str().c_str(), GENERIC_READ | GENERIC_WRITE,
//                                  FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
//    if (INVALID_HANDLE_VALUE == dump_file) {
//        FreeLibrary(dbg_help);
//        return EXCEPTION_CONTINUE_EXECUTION;
//    }
//
//    // 写入 dmp 文件
//    MINIDUMP_EXCEPTION_INFORMATION param;
//    param.ThreadId = GetCurrentThreadId();
//    param.ExceptionPointers = pointers;
//    param.ClientPointers = FALSE;
//
//    dump_writer(GetCurrentProcess(), GetCurrentProcessId(), dump_file, MiniDumpWithDataSegs,
//                (pointers ? &param : NULL), NULL, NULL);
//    // 释放文件
//    CloseHandle(dump_file);
//    FreeLibrary(dbg_help);
//    return 0;
//}
//
//LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS pointers) {
//    // 这里做一些异常的过滤或提示
//    if (IsDebuggerPresent()) {
//        return EXCEPTION_CONTINUE_SEARCH;
//    }
//    return CreateDump(pointers);
//}
//
//DumpWriter::DumpWriter() {}
//
//DumpWriter::~DumpWriter() {}
//
//void DumpWriter::GenerateDumpFile() {
//    SetUnhandledExceptionFilter(ExceptionFilter);
//}
//
//void DumpWriter::SetDumpDir(const std::string& dump_path) {
//    kDumpPath = dump_path;
//}