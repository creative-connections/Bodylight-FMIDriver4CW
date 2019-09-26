#pragma once
#include "libs/qtronic/XmlParser.h"
#include "libs/qtronic/XmlElement.h"
#include "ModelDescriptionFile.hpp"

#include <string>
#include <vector>
#include <unordered_map>

class Fmu
{
public:
  Fmu(const std::wstring & fmuDir, ModelDescriptionFile * modelDescriptionFile, Log * log);
  ~Fmu();

  void getReal(const std::vector<fmi2ValueReference> & references, std::unordered_map<fmi2ValueReference, fmi2Real> & outputs);
  void getInteger(const std::vector<fmi2ValueReference> & references, std::unordered_map<fmi2ValueReference, fmi2Integer> & outputs);
  void getBoolean(const std::vector<fmi2ValueReference> & references, std::unordered_map<fmi2ValueReference, fmi2Boolean> & outputs);

  void setReal(const std::vector<fmi2ValueReference> & references, const std::vector<fmi2Real> & inputs);
  void setInteger(const std::vector<fmi2ValueReference> & references, const std::vector<fmi2Integer> & inputs);
  void setBoolean(const std::vector<fmi2ValueReference> & references, const std::vector<fmi2Boolean> & inputs);

  void setInstanceName(const TValue & value);
  void setInstanceName(const std::string & value);
  std::string getInstanceName();
  void setResourceLocation(const TValue & value);
  void setResourceLocation(const std::string & value);
  void setVisible(const TValue & value);
  void setVisible(const fmi2Boolean & value);
  void setTolerance(const TValue & value);
  void setTolerance(const fmi2Real & value);
  void setStartTime(const TValue & value);
  void setStartTime(const fmi2Real & value);

  void getTime(TValue & value);
  void getLastStepStatus(TValue & value);

  void setFmuLoggingOn();
  void setLogging(const fmi2Boolean & value);

  void instantiate();
  void enterInitializationMode();
  void exitInitializationMode();
  void terminate();
  void reset();
  void doStep(const TValue & stepSize);

  void configurationFinished();

protected:
  Log * log;

  std::string guid;
  std::string modelName;
  std::string modelIdentifier;

  std::string instanceName;
  std::string resourceLocation;
  fmi2Boolean visible = fmi2True;
  fmi2Boolean logging = fmi2True;
  fmi2Boolean toleranceDefined = fmi2False;
  fmi2Real tolerance = 0.0;
  fmi2Real startTime = 0.0;
  fmi2Boolean stopTimeDefined = fmi2False;
  fmi2Real stopTime = 0.0;
  fmi2Component instance = nullptr;

  fmi2Real time = 0.0;
  fmi2Status status = fmi2OK;

  bool instantiated = false;

  FMU fmu;
  ModelDescriptionFile * modelDescriptionFile;

  fmi2CallbackFunctions getCallbackFunctions();
  std::wstring getDllPath(const std::wstring & fmuDir);
  void loadDll(const std::wstring & dllPath);
  void * getAdr(HMODULE dllHandle, const char *functionName);

  void logDescription();
};
