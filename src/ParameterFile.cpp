#include "stdafx.h"
#include "ParameterFile.hpp"

#include <fstream>
#include "Logger.hpp"
#include "wruntime_error.h"

ParameterFile::ParameterFile(unsigned szParamFileName, char * ParamFileName)
{
  // CW8 does not support unicode file paths for ParamFileName
  std::ifstream file;
  file.open(ParamFileName);
  if (!file.is_open()) {
    throw std::runtime_error(std::string("Could not open parameter file (") + ParamFileName + ")" );
  }
  
  // file exists, save its location for future use
  location = Utils::fs::getFolderFromFilePathA(ParamFileName);
  
  // load the file as JSON 
  file >> json;
  file.close();
  
  fillChannels();

  // parameter deserialization
  workingDirectory = jsonPathDir(json["workingDirectory"], location);
  fmu = jsonPathFile(json["fmu"], L"");
  fmuDestinationDirectory = jsonPathDir(json["fmuDestinationDirectory"], L"fmu");

  fmuLoggingOn = jsonBool(json["fmuLoggingOn"], false);
  fmuLogDirectory = jsonPathDir(json["fmuLogDirectory"], L"logs\\fmu\\");
  fmuLogFileName = jsonString(json["fmuLogFileName"], L"fmu");

  driverLoggingOn = jsonBool(json["driverLoggingOn"], false);
  driverLogDirectory = jsonPathDir(json["driverLogDirectory"], L"logs\\driver\\");
  driverLogFileName = jsonString(json["driverLogFileName"], L"CWFmiDriver");
  driverLogVerbosity = jsonString(json["driverLogVerbosity"], L"warning");

  instanceName = jsonString(json["instanceName"], "");
  visible = jsonBool(json["visible"], true);
  resourceLocation = jsonBool(json["resourceLocation"], "file:\\");

  tolerance = jsonFmi2Real(json["tolerance"], -1.0);
  startTime = jsonFmi2Real(json["startTime"], 0.0);
}

ParameterFile::~ParameterFile()
{
}

std::wstring ParameterFile::jsonPathDir(const nlohmann::json & dir, std::wstring defaultValue)
{
  std::wstring path = Utils::fs::sanitizePathDir(defaultValue);
  if (dir.is_string() && dir != "") {
    path = Utils::fs::sanitizePathDir(Utils::enc::utf8ToWstring(dir));
  }
  if (Utils::fs::isRelativePath(path)) {
    return Utils::fs::sanitizePathDir(workingDirectory + path);
  }
  return path;
}

std::wstring ParameterFile::jsonPathFile(const nlohmann::json & file, std::wstring defaultValue)
{
  std::wstring path = Utils::fs::sanitizePathFile(defaultValue);
  if (file.is_string() && file != "") {
    path = Utils::fs::sanitizePathFile(Utils::enc::utf8ToWstring(file));
  }
  if (Utils::fs::isRelativePath(path)) {
    return Utils::fs::sanitizePathFile(workingDirectory + path);
  }
  return path;
}

bool ParameterFile::jsonBool(const nlohmann::json & param, bool defaultValue)
{
  if (param.is_boolean()) {
    return param.get<bool>();
  }
  return defaultValue;
}

std::wstring ParameterFile::jsonString(const nlohmann::json & str, std::wstring defaultValue)
{
  if (str.is_string() && str != "") {
    return Utils::enc::utf8ToWstring(str);
  }
  return defaultValue;
}

std::string ParameterFile::jsonString(const nlohmann::json & str, std::string defaultValue)
{
  if (str.is_string() && str != "") {
    return str;
  }
  return defaultValue;
}

fmi2Real ParameterFile::jsonFmi2Real(const nlohmann::json & real, fmi2Real defaultValue)
{
  if (real.is_number_float()) {
    return real.get<fmi2Real>();
  }
  return defaultValue;
}

void ParameterFile::setLog(Log * log) noexcept
{
  this->log = log;
  DEBUG("Loaded configuration:")
  DEBUG("[workingDirectory       ]: " + Utils::enc::wstringToUtf8(workingDirectory))
  DEBUG("[fmu                    ]: " + Utils::enc::wstringToUtf8(fmu))
  DEBUG("[fmuDestinationDirectory]: " + Utils::enc::wstringToUtf8(fmuDestinationDirectory))
  DEBUG("[fmuLoggingOn           ]: " + std::to_string(fmuLoggingOn))
  DEBUG("[fmuDestinationDirectory]: " + Utils::enc::wstringToUtf8(fmuDestinationDirectory))
  DEBUG("[fmuLogDirectory        ]: " + Utils::enc::wstringToUtf8(fmuLogDirectory))
  DEBUG("[fmuLogFileName         ]: " + Utils::enc::wstringToUtf8(fmuLogDirectory))

  DEBUG("[driverLoggingOn        ]: " + std::to_string(driverLoggingOn))
  DEBUG("[driverLogDirectory     ]: " + Utils::enc::wstringToUtf8(fmuLogDirectory))
  DEBUG("[driverLogFileName      ]: " + Utils::enc::wstringToUtf8(driverLogFileName))
  DEBUG("[driverLogVerbosity     ]: " + Utils::enc::wstringToUtf8(driverLogVerbosity))

  DEBUG("[instanceName           ]: " + instanceName)
  DEBUG("[visible                ]: " + visible)
  DEBUG("[resourceLocation       ]: " + resourceLocation)
  DEBUG("[tolerace               ]: " + std::to_string(tolerance))
  DEBUG("[startTime              ]: " + std::to_string(startTime))

}

void ParameterFile::fillChannels() noexcept
{
  nlohmann::json jsonChannels = json["channels"];
  for (nlohmann::json::iterator it = jsonChannels.begin(); it != jsonChannels.end(); ++it) {
    channelNumber channelNumber = stoul(it.key(), nullptr, 10);
    this->channels[channelNumber] = it.value().get<std::string>();
  }
}

std::unordered_map<channelNumber, TValue> ParameterFile::getInitialValues()
{
  nlohmann::json jsonValues = json["initialValues"];
  std::unordered_map<channelNumber, TValue> initialValues;

  if (jsonValues.is_null() || !jsonValues.is_object()) {
    DEBUG("No initial channel values configured.");
    return initialValues;
  }

  try {
    for (nlohmann::json::iterator it = jsonValues.begin(); it != jsonValues.end(); ++it) {
      channelNumber channelNumber = stoul(it.key(), nullptr, 10);
      auto val = it.value();
      TValue value;
      if (val.is_number_float()) {
        value.Type = vtLongReal;
        value.ValLongReal = val.get<double>();
        initialValues[channelNumber] = value;
        continue;
      }
      if (val.is_number_integer()) {
        value.Type = vtInteger;
        value.ValInteger = static_cast<signed short>(val.get<int64_t>());
        initialValues[channelNumber] = value;
        continue;
      }
      if (val.is_boolean()) {
        value.Type = vtBoolean;
        value.ValBoolean = val.get<bool>();
        initialValues[channelNumber] = value;
        continue;
      }
    }
    return initialValues;
  }
  catch (nlohmann::json::exception & ex) {
    std::wstring msg = L"Exception occured while parsing initial values: " + Utils::enc::acpToWstring(ex.what());
    log->error(msg);
    throw wruntime_error(msg);
  }
}