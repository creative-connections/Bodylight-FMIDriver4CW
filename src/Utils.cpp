#include "stdafx.h"
#include "Utils.h"

#include <iterator>
#include <algorithm>
#include <string>

#include "Logger.hpp"
#include "wruntime_error.h"

#include "Shlwapi.h"
#include "Pathcch.h"

std::wstring Utils::enc::utf8ToWstring(const std::string & in)
{
  int size = MultiByteToWideChar(CP_UTF8, 0, &in[0], (int)in.size(), NULL, 0);
  std::wstring out(size, 0);
  MultiByteToWideChar(CP_UTF8, 0, &in[0], (int)in.size(), &out[0], size);
  return out;
}

std::string Utils::enc::utf8ToAcp(const std::string & in)
{
  // firstly we convert from utf8 to wchar_t, then we convert to ACP 
  // lossy
  return wstringToAcp(utf8ToWstring(in));
}

std::wstring Utils::enc::acpToWstring(const std::string & in)
{
  int size = MultiByteToWideChar(CP_ACP, 0, &in[0], (int)in.size(), NULL, 0);
  std::wstring out(size, 0);
  MultiByteToWideChar(CP_ACP, 0, &in[0], (int)in.size(), &out[0], size);
  return out;
}

std::wstring Utils::enc::acpToWstring(const char * buf)
{
  std::string in(buf);
  return acpToWstring(in);
}

/**
 * Straight wchar_t to char shortening.
 */
std::string Utils::enc::wstringToStringNoConv(const std::wstring & in)
{
  return std::string(in.begin(), in.end());
}

/**
 * Lossy conversion from wchar_t to ACP
 */
std::string Utils::enc::wstringToAcp(const std::wstring & in)
{
  int size = WideCharToMultiByte(CP_ACP, 0, &in[0], (int)in.size(), NULL, 0, NULL, NULL);
  std::string out(size, 0);
  WideCharToMultiByte(CP_ACP, 0, &in[0], (int)in.size(), &out[0], size, NULL, NULL);
  return out;
}

 std::string Utils::enc::wstringToUtf8(const std::wstring & in)
 {
  int size = WideCharToMultiByte(CP_UTF8, 0, &in[0], (int)in.size(), NULL, 0, NULL, NULL);
  std::string out(size, 0);
  WideCharToMultiByte(CP_UTF8, 0, &in[0], (int)in.size(), &out[0], size, NULL, NULL);
  return out;
 }

std::wstring Utils::fs::getFolderFromFilePath(const std::wstring & in)
{
  wchar_t drive[_MAX_DRIVE];
  wchar_t dir[_MAX_DIR];

  _wsplitpath_s(
    in.c_str(),
    drive, _MAX_DRIVE,
    dir, _MAX_DIR,
    NULL, 0,
    NULL, 0
  );
  return std::wstring(drive) + std::wstring(dir);
}

std::wstring Utils::fs::getFolderFromFilePathA(const std::string & in)
{
  char drive[_MAX_DRIVE];
  char dir[_MAX_DIR];

  _splitpath_s(
    in.c_str(),
    drive, _MAX_DRIVE,
    dir, _MAX_DIR,
    NULL, 0,
    NULL, 0
  );
  return Utils::fs::sanitizePathDir(enc::acpToWstring(std::string(drive) + std::string(dir)));
}

std::wstring Utils::fs::getFolderFromFilePathA(const char * in)
{
  return getFolderFromFilePathA(std::string(in));
}

bool Utils::fs::isRelativePath(const std::wstring & path)
{
  wchar_t drive[_MAX_DRIVE];

  _wsplitpath_s(
    path.c_str(),
    drive, _MAX_DRIVE,
    NULL, 0,
    NULL, 0,
    NULL, 0
  );
  return std::wstring(drive) == L"";
}

std::wstring Utils::fs::canonicalize(const std::wstring & in) noexcept
{
  wchar_t out[MAX_PATH];
  wchar_t * path = str::bufferDup(in);

  PathCchCanonicalize(out, MAX_PATH, str::bufferDup(in));

  delete path;
  return std::wstring(out);
}

std::wstring Utils::fs::addTrailingBackslash(const std::wstring & in) noexcept
{
  size_t newsize = in.size() + 2;

  wchar_t * buffer = new wchar_t[newsize];
  std::copy(in.begin(), in.end(), stdext::checked_array_iterator<wchar_t*>(buffer, in.size() + 1));
  buffer[in.size()] = '\0';

  PathCchAddBackslash(buffer, newsize);

  return buffer;
}

std::wstring Utils::fs::replaceForwardSlashes(const std::wstring & in) noexcept
{
  std::wstring out(in);
  std::replace(out.begin(), out.end(), '/', '\\');
  return out;
}

std::wstring Utils::fs::getCurrentDirectory()
{
  wchar_t currentDir[MAX_PATH];
  GetCurrentDirectoryW(MAX_PATH, currentDir);
  return std::wstring(currentDir);
}

std::wstring Utils::fs::sanitizePathFile(const std::wstring & path)
{
  return fs::canonicalize(fs::replaceForwardSlashes(path));
}

std::wstring Utils::fs::sanitizePathDir(const std::wstring & path)
{
  return fs::canonicalize(
    fs::addTrailingBackslash(
      fs::replaceForwardSlashes(path)
    )
  );
}

bool Utils::fs::isDirectory(const std::wstring & dirpath) noexcept
{
  DWORD attr = GetFileAttributesW(dirpath.c_str());

  // path does not exist
  if (attr == INVALID_FILE_ATTRIBUTES) {
    return false;
  }
  
  return ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0) || ((attr & FILE_ATTRIBUTE_REPARSE_POINT) != 0);
}

bool Utils::fs::isFile(const std::wstring & filepath) noexcept
{
  DWORD attr = GetFileAttributesW(filepath.c_str());

  // path does not exist
  if (attr == INVALID_FILE_ATTRIBUTES) {
    return false;
  }
  
  return (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool Utils::fs::createDirectory(const std::wstring & dirpath) noexcept
{
  wchar_t * path = str::bufferDup(sanitizePathDir(dirpath));
  wchar_t folder[MAX_PATH];
  wchar_t * end;

  end = wcschr(path, L'\\');

  bool success = true; 
  while (end != NULL) {
    wcsncpy_s(folder, path, end - path + 1);
    if (!isDirectory(folder)) {
      if (!CreateDirectoryW(folder, NULL)) {
        if (GetLastError() == ERROR_PATH_NOT_FOUND) {
          success = false;
          break;
        }
      }
    }
    end = wcschr(++end, L'\\');
  }

  delete path;
  return success;
}

wchar_t * Utils::str::bufferDup(const std::wstring & in)
{
  wchar_t * buffer = new wchar_t[in.size() + 1];
  std::copy(in.begin(), in.end(), stdext::checked_array_iterator<wchar_t*>(buffer, in.size() + 1));
  buffer[in.size()] = '\0';
  return buffer;
}

char * Utils::str::bufferDup(const std::string & in)
{
  char * buffer = new char[in.size() + 1];
  std::copy(in.begin(), in.end(), stdext::checked_array_iterator<char*>(buffer, in.size() + 1));
  buffer[in.size()] = '\0';
  return buffer;
}

std::string Utils::tvalue::getString(const TValue & value, const std::string & defaultValue) noexcept
{
  if (value.Type == vtDriverString) {
    return std::string(static_cast<char*>(value.PString), value.StringCharLength);
  }
  // TODO: log warning wrong type 
  return defaultValue;
}

double Utils::tvalue::getReal(const TValue & value, const double & defaultValue) noexcept
{
  if (value.Type == vtLongReal) {
    return value.ValLongReal;
  }
  // TODO: log warning wrong type 
  return defaultValue;
}

bool Utils::tvalue::getBoolean(const TValue & value, const bool & defaultValue) noexcept
{
  if (value.Type == vtBoolean) {
    return value.ValBoolean;
  }
  // TODO: log warning wrong type 
  return defaultValue;
}

fmi2Real Utils::tvalue::getFmi2Real(const TValue & value)
{
  if (value.Type == vtLongReal) {
    return value.ValLongReal;
  }
  throw wruntime_error(L"Value is not real, instead we saw TValue type " + std::to_wstring(value.Type));
}

fmi2Integer Utils::tvalue::getFmi2Integer(const TValue & value)
{
  if (value.Type == vtInteger) {
    return value.ValInteger;
  }
  throw wruntime_error(L"Value is not integer, instead we saw TValue type " + std::to_wstring(value.Type));
}

fmi2Boolean Utils::tvalue::getFmi2Boolean(const TValue & value)
{
  if (value.Type == vtBoolean) {
    return value.ValBoolean;
  }
  throw wruntime_error(L"Value is not boolean, instead we saw TValue type " + std::to_wstring(value.Type));
}

