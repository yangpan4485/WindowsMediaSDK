#pragma once
#include <cstdint>

class VersionHelper {
public:
    static VersionHelper& Instance();

    bool IsWindows10FallCreatorOrLater() const;
    bool IsWindows7() const;
    bool IsWindows8Or8point1() const;
    bool IsWindows8OrLater() const;
    bool IsWindows10OrLater() const;

    int GetMajorVersion() const;
    int GetMinorVersion() const;
    int GetBuildNumber() const;

private:
    VersionHelper();
    ~VersionHelper();
    VersionHelper(const VersionHelper&) = delete;
    VersionHelper operator =(const VersionHelper&) = delete;
    
protected:
    int32_t major_version_{};
    int32_t minor_version_{};
    int32_t build_number_{};
};