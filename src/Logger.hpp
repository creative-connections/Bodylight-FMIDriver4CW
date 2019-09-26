#pragma once

#include "libs/spdlog/spdlog.h"

#define DEBUG(X) if(log->debugEnabled) {log->debug(X);}
#define TRACE(X) if(log->traceEnabled) {log->trace(X);}

class Log
{
public:
  Log();
  ~Log();

  static std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> fmuLoggers;
  static void fmuLoggingFunction(fmi2Component m, fmi2String instanceName, fmi2Status status, fmi2String category, fmi2String message, ...);

  void configureDriverLogger(const bool & enabled, const std::wstring & dir, const std::wstring & fileName, const std::wstring & verbosity);
  void configureFmuLogger(const bool & enabled, const std::wstring & dir, const std::wstring & fileName, const std::string & instanceName);

  void error(const std::string & msg) noexcept;
  void error(const std::wstring & msg) noexcept;
  void warn(const std::string & msg) noexcept;
  void warn(const std::wstring & msg) noexcept;
  void debug(const std::string & msg) noexcept;
  void debug(const std::wstring & msg) noexcept;
  void trace(const std::string & msg) noexcept;
  void trace(const std::wstring & msg) noexcept;

  bool debugEnabled = false;
  bool traceEnabled = false;

protected: 
  std::shared_ptr<spdlog::logger> driverLog;

  std::string instanceName;

  bool driverLoggingOn = false;

  spdlog::level::level_enum parseLevel(const std::wstring & in) noexcept;
  std::wstring getTimestamp() noexcept;
};
