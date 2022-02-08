#include "version_helper.h"

#include <Windows.h>

VersionHelper& VersionHelper::Instance() {
    static VersionHelper s_instance;
    return s_instance;
}

VersionHelper::VersionHelper() {
    DWORD major_ver = 0;
    DWORD minor_ver = 0;
    DWORD build_number = 0;
    HMODULE module{};
    if (module = ::LoadLibraryW(L"ntdll.dll")) {
        typedef void(WINAPI * pfRTLGETNTVERSIONNUMBERS)(DWORD*, DWORD*, DWORD*);
        pfRTLGETNTVERSIONNUMBERS RtlGetNtVersionNumbers =
            (pfRTLGETNTVERSIONNUMBERS)::GetProcAddress(module, "RtlGetNtVersionNumbers");
        if (RtlGetNtVersionNumbers) {
            RtlGetNtVersionNumbers(&major_ver, &minor_ver, &build_number);
            build_number &= 0x0ffff;
        }
        ::FreeLibrary(module);
        module = NULL;
    }
    major_version_ = major_ver;
    minor_version_ = minor_ver;
    build_number_ = build_number;
}

VersionHelper::~VersionHelper() {}

int VersionHelper::GetMajorVersion() const {
    return major_version_;
}

int VersionHelper::GetMinorVersion() const {
    return minor_version_;
}

int VersionHelper::GetBuildNumber() const {
    return build_number_;
}

bool VersionHelper::IsWindows10FallCreatorOrLater() const {
    if (major_version_ > 10) {
        return true;
    } else if (major_version_ == 10 && minor_version_ > 0) {
        return true;
    } else if (major_version_ == 10 && minor_version_ == 0 && build_number_ >= 16299) {
        return true;
    }
    return false;
}

bool VersionHelper::IsWindows7() const {
    return (major_version_ == 6 && minor_version_ == 1);
}

bool VersionHelper::IsWindows8Or8point1() const {
    return (major_version_ == 6 && minor_version_ > 1);
}

bool VersionHelper::IsWindows8OrLater() const {
    if (major_version_ > 6) {
        return true;
    } else if (major_version_ == 6 && minor_version_ >= 2) {
        return true;
    }
    return false;
}

bool VersionHelper::IsWindows10OrLater() const {
    return major_version_ >= 10;
}