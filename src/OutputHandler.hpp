#pragma once

#include "Fmu.hpp"
#include "ParameterFile.hpp"
#include "ModelDescriptionFile.hpp"

#include <unordered_map>

class OutputHandler
{
public:
  OutputHandler(
    Fmu * fmu, 
    Log * log,
    std::unordered_map<channelNumber, std::pair<fmi2ValueReference, fmi2DataType>> channelMapping, 
    HANDLE CBHandle,
    const tDriverCallback driverCallback
  );

  ~OutputHandler() {};

  void OutputRequest(const channelNumber & ChannelNumber, const TValue & ChannelValue);
  void OutputRequestCompleted();
  char OutputFinalized(const channelNumber & ChannelNumber, unsigned * ErrorCode);

  void flush(bool notify = true);
protected:
  Fmu * fmu;
  Log * log;
  tDriverCallback driverCallback;
  HANDLE CBHandle;

  std::unordered_map<channelNumber, std::pair<fmi2ValueReference, fmi2DataType>> channelMapping;

  std::vector<fmi2ValueReference> real;
  std::vector<fmi2ValueReference> integer;
  std::vector<fmi2ValueReference> boolean;

  std::vector<fmi2Real> realInput;
  std::vector<fmi2Integer> integerInput;
  std::vector<fmi2Boolean> booleanInput;

  std::vector<channelNumber> toCallback;
  std::unordered_map<fmi2ValueReference, unsigned> errors;

  bool empty = true;

};

