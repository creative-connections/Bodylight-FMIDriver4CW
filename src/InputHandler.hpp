#pragma once

#include "stdafx.h"

#include <vector>
#include <map>
#include "Fmu.hpp"
#include "ParameterFile.hpp"
#include "ModelDescriptionFile.hpp"

class InputHandler
{
public:
  InputHandler(
    Fmu * fmu, 
    Log * log,
    std::unordered_map<channelNumber, std::pair<fmi2ValueReference, fmi2DataType>> channelMapping, 
    HANDLE CBHandle,
    const tDriverCallback driverCallback
  );
  ~InputHandler() {};

  void InputRequest(const channelNumber & ChannelNumber);
  void InputRequestCompleted();
  char InputFinalized(const channelNumber & ChannelNumber, unsigned * ErrorCode);
  void GetInput(const channelNumber & ChannelNumber, TValue * ChannelValue);

protected:
  Fmu * fmu;
  Log * log;
  tDriverCallback driverCallback;
  HANDLE CBHandle;

  std::unordered_map<channelNumber, std::pair<fmi2ValueReference, fmi2DataType>> channelMapping;

  std::vector<fmi2ValueReference> real;
  std::vector<fmi2ValueReference> integer;
  std::vector<fmi2ValueReference> boolean;

  std::unordered_map<fmi2ValueReference, fmi2Real> realOutput;
  std::unordered_map<fmi2ValueReference, fmi2Integer> integerOutput;
  std::unordered_map<fmi2ValueReference, fmi2Boolean> booleanOutput;

};

