#include "stdafx.h"

#include <time.h>

#include "Logger.hpp"
#include "ParameterFile.hpp"

std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> Log::fmuLoggers;

Log::Log()
{

}

Log::~Log()
{
  fmuLoggers.erase(instanceName);
}

void Log::fmuLoggingFunction(fmi2Component m, fmi2String instanceName, fmi2Status status, fmi2String category, fmi2String message, ...)
{
  std::shared_ptr<spdlog::logger> fmuLog = fmuLoggers.find(instanceName)->second;

  va_list argptr;
  va_start(argptr, message);

  char * buf = new char[1];
  int size = vsnprintf(buf, 0, message, argptr);
  delete[] buf;

  buf = new char[size];
  vsnprintf(buf, size, message, argptr);

  fmuLog->info(" [" + std::string(instanceName) + "] [" + std::to_string(status) + "] [" + category + "] " + std::string(buf));

  delete[] buf;
  va_end(argptr);
}

void Log::configureDriverLogger(const bool & enabled, const std::wstring & dir, const std::wstring & fileName, const std::wstring & verbosity)
{
  if (enabled) {
    try {
      if (Utils::fs::createDirectory(dir)) {
        driverLoggingOn = true;
        std::wstring timestamp = getTimestamp();
        std::wstring fileStamped = dir + fileName + timestamp;
        // TODO: check spdlog for utf support
        driverLog = spdlog::basic_logger_mt(
          Utils::enc::wstringToUtf8(fileName),
          Utils::enc::wstringToUtf8(fileStamped)
        );

        driverLog->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        spdlog::level::level_enum level = parseLevel(verbosity);
        driverLog->set_level(level);
        driverLog->flush_on(spdlog::level::warn);
      }
    }
    catch (const spdlog::spdlog_ex &ex) {
      driverLoggingOn = false;
      throw wruntime_error(L"Driver logger error: " + Utils::enc::utf8ToWstring(ex.what()));
    }
  }
}

void Log::configureFmuLogger(const bool & enabled, const std::wstring & dir, const std::wstring & fileName, const std::string & instanceName)
{
  if (enabled) {
    try {
      if (Utils::fs::createDirectory(dir)) {
        if (fmuLoggers.find(instanceName) != fmuLoggers.end()) {
          error("FMU log for " + instanceName + " already exists");
          error("Driver instances MUST have different names");
          return;
        }

        // TODO: timestamp
        std::wstring timestamp = getTimestamp();
        std::wstring fileStamped = dir + fileName + timestamp;
        // TODO: check spdlog for utf support
        std::shared_ptr<spdlog::logger> fmuLog = spdlog::basic_logger_mt(
          instanceName, // logger name to be retrieved after
          Utils::enc::wstringToUtf8(fileStamped)
        );

        fmuLog->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %v");
        fmuLog->set_level(spdlog::level::info);

        fmuLoggers[instanceName] = fmuLog;
        this->instanceName = instanceName;
      }
    }
    catch (const spdlog::spdlog_ex &ex) {
      throw wruntime_error(L"Fmu logger error: " + Utils::enc::utf8ToWstring(ex.what()));
    }
  }
}

void Log::error(const std::string & msg) noexcept
{
  if (driverLoggingOn) {
    driverLog->error(msg);
  }
}

void Log::error(const std::wstring & msg) noexcept
{
  if (driverLoggingOn) {
    driverLog->error(Utils::enc::wstringToUtf8(msg));
  }
}

void Log::warn(const std::string & msg) noexcept
{
  if (driverLoggingOn) {
    driverLog->warn(msg);
  }
}

void Log::warn(const std::wstring & msg) noexcept
{
  if (driverLoggingOn) {
    driverLog->warn(Utils::enc::wstringToUtf8(msg));
  }
}

void Log::debug(const std::string & msg) noexcept
{
  if (driverLoggingOn) {
    driverLog->debug(msg);
  }
}

void Log::debug(const std::wstring & msg) noexcept
{
  if (driverLoggingOn) {
    driverLog->debug(Utils::enc::wstringToUtf8(msg));
  }
}

void Log::trace(const std::string & msg) noexcept
{
  if (driverLoggingOn) {
    driverLog->trace(msg);
  }
}

void Log::trace(const std::wstring & msg) noexcept
{
  if (driverLoggingOn) {
    driverLog->trace(Utils::enc::wstringToUtf8(msg));
  }
}

spdlog::level::level_enum Log::parseLevel(const std::wstring & in) noexcept
{
  if (in == L"error") {
    return spdlog::level::err;
  }
  if (in == L"warning") {
    return spdlog::level::warn;
  }
  if (in == L"debug") {
    debugEnabled = true;
    return spdlog::level::debug;
  }
  if (in == L"trace") {
    debugEnabled = true;
    traceEnabled = true;
    return spdlog::level::trace;
  }
  return spdlog::level::warn;
}

std::wstring Log::getTimestamp() noexcept
{
  time_t rawtime = time(NULL);
  struct tm timeinfo;
  char buffer[17];

  time(&rawtime);
  localtime_s(&timeinfo, &rawtime);

  strftime(buffer, 17, "_%Y%m%d_%H%M%S", &timeinfo);
  puts(buffer);

  std::wstring out = Utils::enc::acpToWstring(std::string(buffer));
  return out;
}

