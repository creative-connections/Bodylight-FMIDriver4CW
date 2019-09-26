#pragma once

#include "stdafx.h"
#include "libs/nlohmann/json.hpp"

#include <unordered_map>

class ParameterFile
{
public:
  ParameterFile(unsigned szParamFileName, char * ParamFileName);
  ~ParameterFile();

  void setLog(Log * log) noexcept;

  std::unordered_map<channelNumber, std::string> channels;

  std::unordered_map<channelNumber, TValue> getInitialValues();

  std::wstring workingDirectory;
  std::wstring fmu;
  std::wstring fmuDestinationDirectory;
  
  bool fmuLoggingOn;
  std::wstring fmuLogDirectory;
  std::wstring fmuLogFileName;
  bool driverLoggingOn;
  std::wstring driverLogDirectory;
  std::wstring driverLogFileName;
  std::wstring driverLogVerbosity;

  std::string instanceName;
  bool visible;
  std::string resourceLocation;

  fmi2Real tolerance;
  fmi2Real startTime;

protected:
  nlohmann::json json;
  std::wstring location;

  Log * log;

  void fillChannels() noexcept;

  std::wstring jsonPathDir(const nlohmann::json & dir, std::wstring defaultValue);
  std::wstring jsonPathFile(const nlohmann::json & file, std::wstring defaultValue);
  bool jsonBool(const nlohmann::json & param, bool defaultValue);
  std::wstring jsonString(const nlohmann::json & str, std::wstring defaultValue);
  std::string jsonString(const nlohmann::json & str, std::string defaultValue);
  fmi2Real jsonFmi2Real(const nlohmann::json & real, fmi2Real defaultValue);
};

