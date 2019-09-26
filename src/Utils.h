#pragma once

#include <unordered_map>

namespace Utils {
  namespace enc {
    std::wstring utf8ToWstring(const std::string & in);
    std::string utf8ToAcp(const std::string & in);
    std::wstring acpToWstring(const std::string & in);
    std::wstring acpToWstring(const char * buf);
    std::string wstringToStringNoConv(const std::wstring & in);
    std::string wstringToAcp(const std::wstring & in);
    std::string wstringToUtf8(const std::wstring & in);
  }

  namespace fs {
    std::wstring getFolderFromFilePath(const std::wstring & in);
    std::wstring getFolderFromFilePathA(const std::string & in);
    std::wstring getFolderFromFilePathA(const char * in);

    bool isRelativePath(const std::wstring & path);

    std::wstring canonicalize(const std::wstring & in) noexcept;
    std::wstring addTrailingBackslash(const std::wstring & in) noexcept;
    std::wstring replaceForwardSlashes(const std::wstring & in) noexcept;

    std::wstring getCurrentDirectory();
    std::wstring sanitizePathFile(const std::wstring & path);
    std::wstring sanitizePathDir(const std::wstring & path);

    bool isDirectory(const std::wstring & dirpath) noexcept;
    bool isFile(const std::wstring & filepath) noexcept;
    bool createDirectory(const std::wstring & dirpath) noexcept;
  }

  namespace str {
    wchar_t * bufferDup(const std::wstring & in);
    char * bufferDup(const std::string & in);
  }

  namespace tvalue {
    std::string getString(const TValue & value, const std::string & defaultValue) noexcept;
    double getReal(const TValue & value, const double & defaultValue) noexcept;
    bool getBoolean(const TValue & value, const bool & defaultValue) noexcept;
    fmi2Real getFmi2Real(const TValue & value);
    fmi2Integer getFmi2Integer(const TValue & value);
    fmi2Boolean getFmi2Boolean(const TValue & value);
  }
}
