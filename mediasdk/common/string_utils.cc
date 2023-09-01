#include "string_utils.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <wchar.h>
#include <windows.h>
#include <vector>
#include <memory>

namespace utils
{
    std::string UnicodeToUtf8(const std::wstring& unicode)
    {
        return UnicodeToUtf8(unicode.data(), unicode.length());
    }

    std::string UnicodeToUtf8(const wchar_t* unicode)
    {
        return UnicodeToUtf8(unicode, wcslen(unicode));
    }

    std::string UnicodeToUtf8(const wchar_t* unicode, size_t len)
    {
        if (len == 0) {
            return std::string();
        }
        int len8 = ::WideCharToMultiByte(CP_UTF8, 0, unicode, static_cast<int>(len), nullptr, 0, nullptr, nullptr);
        std::string ns(len8, 0);
        ::WideCharToMultiByte(CP_UTF8, 0, unicode, static_cast<int>(len), &*ns.begin(), len8, nullptr, nullptr);
        return ns;
    }

    std::wstring Utf8ToUnicode(const std::string& utf8)
    {
        std::wstring wstrUnicode{};
        int widesize = ::MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
        if (widesize == ERROR_NO_UNICODE_TRANSLATION || widesize == 0)
        {
            return wstrUnicode;
        }

        std::vector<wchar_t> unicode(widesize);
        int convresult = ::MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &unicode[0], widesize);
        if (convresult != widesize)
        {
            return wstrUnicode;
        }
        wstrUnicode = std::wstring(&unicode[0]);
        return wstrUnicode;
    }

    std::string UnicodeToAcsii(const std::wstring& unicode)
    {
        int asciiSize = ::WideCharToMultiByte(CP_OEMCP, 0, unicode.c_str(), -1, NULL, 0, NULL, NULL);
        if (asciiSize == ERROR_NO_UNICODE_TRANSLATION || asciiSize == 0)
        {
            return "";
        }

        std::vector<char> utf8(asciiSize);
        int convresult = ::WideCharToMultiByte(CP_OEMCP, 0, unicode.c_str(), -1, &utf8[0], asciiSize, NULL, NULL);
        if (convresult != asciiSize)
        {
            return "";
        }
        return std::string(&utf8[0]);
    }

    std::wstring AcsiiToUnicode(const std::string& ascii)
    {
        int widesize = MultiByteToWideChar(CP_ACP, 0, (char*)ascii.c_str(), -1, NULL, 0);
        if (widesize == ERROR_NO_UNICODE_TRANSLATION)
        {
            throw std::exception("Invalid UTF-8 sequence.");
        }
        if (widesize == 0)
        {
            throw std::exception("Error in conversion.");
        }
        std::vector<wchar_t> unicode(widesize);
        int convresult = MultiByteToWideChar(CP_ACP, 0, (char*)ascii.c_str(), -1, &unicode[0], widesize);


        if (convresult != widesize)
        {
            throw std::exception("La falla!");
        }

        return std::wstring(&unicode[0]);
    }

    std::string UnicodeToAnsi(const std::wstring& unicode)
    {
        std::string ret;
        std::mbstate_t state = {};
        const wchar_t *src = unicode.data();
        size_t len = std::wcsrtombs(nullptr, &src, 0, &state);
        if (static_cast<size_t>(-1) != len) {
            std::unique_ptr<char[]> buff(new char[len + 1]);
            len = std::wcsrtombs(buff.get(), &src, len, &state);
            if (static_cast<size_t>(-1) != len) {
                ret.assign(buff.get(), len);
            }
        }
        return ret;
    }

    std::wstring AnsiToUnicode(const std::string& ansi)
    {
        std::wstring ret;
        std::mbstate_t state = {};
        const char *src = ansi.data();
        size_t len = std::mbsrtowcs(nullptr, &src, 0, &state);
        if (static_cast<size_t>(-1) != len) {
            std::unique_ptr< wchar_t[] > buff(new wchar_t[len + 1]);
            len = std::mbsrtowcs(buff.get(), &src, len, &state);
            if (static_cast<size_t>(-1) != len) {
                ret.assign(buff.get(), len);
            }
        }
        return ret;
    }

    std::string Utf8ToAnsi(const std::string& utf8)
    {
        std::wstring unicode = Utf8ToUnicode(utf8);
        return UnicodeToAcsii(unicode);
    }

    std::string AnsiToUtf8(const std::string& ansi)
    {
        std::wstring unicode = AnsiToUnicode(ansi);
        return UnicodeToUtf8(unicode);
    }

    std::string Utf8ToAscii(const std::string& utf8)
    {
        std::wstring unicode = Utf8ToUnicode(utf8);
        return UnicodeToAcsii(unicode);
    }

    std::string AsciiToUtf8(const std::string& ascii)
    {
        std::wstring unicode = AcsiiToUnicode(ascii);
        return UnicodeToUtf8(unicode);
    }

    std::string AnsiToAscii(const std::string& ansi)
    {
        std::wstring unicode = AnsiToUnicode(ansi);
        return UnicodeToAcsii(unicode);
    }

    std::string AsciiToAnsi(const std::string& ascii)
    {
        std::wstring unicode = AcsiiToUnicode(ascii);
        return UnicodeToAnsi(unicode);
    }
}
