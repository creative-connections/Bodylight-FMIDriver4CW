#include "stdafx.h"
#include "OutputHandler.hpp"

#include "Logger.hpp"

OutputHandler::OutputHandler(Fmu * fmu, Log * log, std::unordered_map<channelNumber, std::pair<fmi2ValueReference, fmi2DataType>> channelMapping, HANDLE CBHandle, const tDriverCallback driverCallback)
  :fmu(fmu), log(log), driverCallback(driverCallback), CBHandle(CBHandle), channelMapping(channelMapping)
{
}

void OutputHandler::OutputRequest(const channelNumber & ChannelNumber, const TValue & ChannelValue)
{
  auto it = channelMapping.find(ChannelNumber);
  if (it == channelMapping.end()) {
    log->error("Unknown output channel " + std::to_string(ChannelNumber));
    return;
  }
  const std::pair<fmi2ValueReference, fmi2DataType> & mapping = it->second;
  // TODO: report custom error for read only FMU variables

  // exceptions get generated at Utils::tvalue methods, on exception we received
  // unexpected value == channel configuration is wrong.
  try {
    switch (mapping.second) {
    case FMI_REAL:
      realInput.push_back(Utils::tvalue::getFmi2Real(ChannelValue));
      real.push_back(mapping.first);
      TRACE("Output channel: " + std::to_string(ChannelNumber) + " [r] " + std::to_string(Utils::tvalue::getFmi2Real(ChannelValue)));
      break;
    case FMI_INTEGER:
      integerInput.push_back(Utils::tvalue::getFmi2Integer(ChannelValue));
      integer.push_back(mapping.first);
      TRACE("Output channel: " + std::to_string(ChannelNumber) + " [b] " + std::to_string(Utils::tvalue::getFmi2Integer(ChannelValue)));
      break;
    case FMI_BOOLEAN:
      booleanInput.push_back(Utils::tvalue::getFmi2Boolean(ChannelValue));
      // this is correct, since fmi2Boolean is an integer, no vector issue here
      boolean.push_back(mapping.first);
      TRACE("Output channel: " + std::to_string(ChannelNumber) + " [b] " + std::to_string(Utils::tvalue::getFmi2Boolean(ChannelValue)));
      break;
    }
  }
  catch (const wruntime_error & ex) {
    log->error("Exception occured on output channel " + std::to_string(ChannelNumber) + ": " + ex.what());
    errors[mapping.first] = ceValue;
  }
}

void OutputHandler::OutputRequestCompleted()
{
  empty = false;
}

char OutputHandler::OutputFinalized(const channelNumber & ChannelNumber, unsigned * ErrorCode)
{
  auto it = channelMapping.find(ChannelNumber);
  if (it == channelMapping.end()) {
    log->error("Unknown output channel " + std::to_string(ChannelNumber) + " reported to ControlWeb.");
    *ErrorCode = ceUnknownChannel;
    return 0;
  }

  const std::pair<fmi2ValueReference, fmi2DataType> & mapping = it->second;

  // check if there is an error code logged for this fmi2ValueReference
  auto iserror = errors.find(mapping.first);
  if (iserror != errors.end()) {
    *ErrorCode = iserror->second;
    log->error(L"Could not set Channel " + std::to_wstring(ChannelNumber) + L" error: " + std::to_wstring(iserror->second));
    errors.erase(iserror->first);
    return 0;
  }

  *ErrorCode = ceOK;
  if (empty) {
    // This occurs when flush(fmi2DoStep) happens during OutputFinalized querying stage.
    TRACE("Output for channel: " + std::to_string(ChannelNumber) + " is set.");
    return 1;
  }
  else {
    TRACE("Output for channel: " + std::to_string(ChannelNumber) + " is buffered.");
    return 0;
  }
}

/**
  \note
  flush() should get called right before fmu2DoStep, since some FMUs really dislike
  if fmi2Get* gets called after fmi2Set*.
*/
void OutputHandler::flush(bool notify)
{
  if (empty) {
    return;
  }

  if (real.size() > 0) {
    fmu->setReal(real, realInput);
    real.clear();
    realInput.clear();
    TRACE("Flushed all REAL outputs to FMU");
  }

  if (integer.size() > 0) {
    fmu->setInteger(integer, integerInput);
    integer.clear();
    integerInput.clear();
    TRACE("Flushed all INTEGER outputs to FMU");
  }

  if (boolean.size() > 0) {
    fmu->setBoolean(boolean, booleanInput);
    boolean.clear();
    booleanInput.clear();
    TRACE("Flushed all BOOLEAN outputs to FMU");
  }

  if (notify) {
    unsigned ret = (*driverCallback)(CBHandle, dcfDriverOutputFinalized, nullptr);
    TRACE("Notified ControlWeb about the output flush, ControlWeb response: " + std::to_string(ret));
  }

  empty = true;
}
