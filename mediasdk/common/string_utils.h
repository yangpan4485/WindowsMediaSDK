#pragma once

#include <cstdint>
#include <string>

namespace utils
{
    std::string UnicodeToUtf8(const std::wstring& unicode);
    std::string UnicodeToUtf8(const wchar_t* unicode);
    std::string UnicodeToUtf8(const wchar_t* unicode, size_t len);
    std::wstring Utf8ToUnicode(const std::string& utf8);

    std::string UnicodeToAcsii(const std::wstring& unicode);
    std::wstring AcsiiToUnicode(const std::string& ascii);

    std::string UnicodeToAnsi(const std::wstring& unicode);
    std::wstring AnsiToUnicode(const std::string& ansi);

    std::string Utf8ToAnsi(const std::string& utf8);
    std::string AnsiToUtf8(const std::string& ansi);

    std::string Utf8ToAscii(const std::string& utf8);
    std::string AsciiToUtf8(const std::string& ascii);

    std::string AnsiToAscii(const std::string& ansi);
    std::string AsciiToAnsi(const std::string& ascii);
}
