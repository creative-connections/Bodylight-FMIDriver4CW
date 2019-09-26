#include "stdafx.h"
#include "InputHandler.hpp"

#include "Logger.hpp"


InputHandler::InputHandler(Fmu * fmu, Log * log, std::unordered_map<channelNumber, std::pair<fmi2ValueReference, fmi2DataType>> channelMapping, HANDLE CBHandle, const tDriverCallback driverCallback) 
  :fmu(fmu), log(log), driverCallback(driverCallback), CBHandle(CBHandle), channelMapping(channelMapping)
{
}

void InputHandler::InputRequest(const channelNumber & ChannelNumber)
{
  auto it = channelMapping.find(ChannelNumber);
  if (it == channelMapping.end()) {
    log->error("Unknown input channel " + std::to_string(ChannelNumber));
    return;
  }
  const std::pair<fmi2ValueReference, fmi2DataType> & mapping = it->second;

  switch (mapping.second) {
  case FMI_REAL: 
    real.push_back(mapping.first);
    break;
  case FMI_INTEGER:
    integer.push_back(mapping.first);
    break;
  case FMI_BOOLEAN:
    boolean.push_back(mapping.first);
    break;
  case FMI_STRING:
    // not supported, error will get thrown on InputFinalized
    break;
  }
}

void InputHandler::InputRequestCompleted()
{
  TRACE("InputRequestCompleted called");
  if (real.size() > 0) {
    fmu->getReal(real, realOutput);
    real.clear();
    TRACE("Retrieved REAL values from the FMU");
  }

  if (integer.size() > 0) {
    fmu->getInteger(integer, integerOutput);
    integer.clear();
    TRACE("Retrieved INTEGER values from the FMU");
  }

  if (boolean.size() > 0) {
    fmu->getBoolean(boolean, booleanOutput);
    boolean.clear();
    TRACE("Retrieved BOOL values from the FMU");
  }
}

char InputHandler::InputFinalized(const channelNumber & ChannelNumber, unsigned * ErrorCode)
{
  auto it = channelMapping.find(ChannelNumber);
  if (it == channelMapping.end()) {
    log->error("Unknown input channel " + std::to_string(ChannelNumber));
    *ErrorCode = ceUnknownChannel;
    return 0;
  }
  if (it->second.second == FMI_STRING) {
    log->error("Channel " + std::to_string(ChannelNumber) + " requested a string input, strings are not supported.");
    *ErrorCode = ceValue;
    return 0;
  }

  *ErrorCode = ceOK;
  return 1;
}

void InputHandler::GetInput(const channelNumber & ChannelNumber, TValue * ChannelValue)
{
  std::pair<fmi2ValueReference, fmi2DataType> & mapping = channelMapping[ChannelNumber];

  switch (mapping.second) {
  case FMI_REAL:
    ChannelValue->ValLongReal = realOutput[mapping.first];
    TRACE("Input channel: " + std::to_string(ChannelNumber) + " [r] " + std::to_string(ChannelValue->ValLongReal))
    break;
  case FMI_INTEGER:
    ChannelValue->ValLongInt = integerOutput[mapping.first];
    TRACE("Input channel: " + std::to_string(ChannelNumber) + " [i] " + std::to_string(ChannelValue->ValLongInt))
    break;
  case FMI_BOOLEAN:
    ChannelValue->ValBoolean = booleanOutput[mapping.first];
    TRACE("Input channel: " + std::to_string(ChannelNumber) + " [b] " + std::to_string(ChannelValue->ValBoolean))
    break;
  }
}
