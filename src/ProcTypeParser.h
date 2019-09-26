#pragma once

typedef enum ProcType {
  setInstanceName,
  setVisible,
  setResourceLocation,
  setTolerance,
  setStartTime,
  initialize,
  doStep,
  setFmuLoggingOn,
  terminateFmu,
  getTime,
  resetFmu,
  getLastStepStatus,
  unknown,
} ProcType;

class ProcTypeParser
{
public:
  ProcType parse(const char * type);
  ProcType parse(const char * type, size_t size);

  ProcTypeParser()
  {
    procTypeMap["setInstanceName"] = ProcType::setInstanceName;
    procTypeMap["setVisible"] = ProcType::setVisible;
    procTypeMap["setResourceLocation"] = ProcType::setResourceLocation;
    procTypeMap["setTolerance"] = ProcType::setTolerance;
    procTypeMap["setStartTime"] = ProcType::setStartTime;
    procTypeMap["initialize"] = ProcType::initialize;
    procTypeMap["setFmuLoggingOn"] = ProcType::setFmuLoggingOn;
    procTypeMap["doStep"] = ProcType::doStep;
    procTypeMap["terminate"] = ProcType::terminateFmu;
    procTypeMap["reset"] = ProcType::resetFmu;
    procTypeMap["getLastStepStatus"] = ProcType::getLastStepStatus;
    procTypeMap["getTime"] = ProcType::getTime;
  }
  ~ProcTypeParser() { };

protected:
  std::unordered_map<std::string, ProcType> procTypeMap;
};
