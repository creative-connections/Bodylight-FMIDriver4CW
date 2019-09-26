#pragma once

#include "Logger.hpp"
#include "InputHandler.hpp"
#include "OutputHandler.hpp"
#include "ParameterFile.hpp"
#include "Fmu.hpp"
#include "ProcTypeParser.h"

#include <stdexcept>
#include <vector>

class Driver
{
public:
  ~Driver();
  char Init(unsigned szParamFileName, char * ParamFileName, unsigned szInitMessage, char * InitMessage, unsigned LoginLevel, char RunningFlag, HANDLE CBHandle, tDriverCallback DriverCallback);

  void InputRequest(const channelNumber & ChannelNumber);
  void InputRequestCompleted();
  char InputFinalized(const channelNumber & ChannelNumber, unsigned * ErrorCode);
  void GetInput(const channelNumber & ChannelNumber, TValue * ChannelValue);

  void OutputRequest(const channelNumber & ChannelNumber, const TValue & ChannelValue);
  void OutputRequestCompleted();
  char OutputFinalized(const channelNumber & ChannelNumber, unsigned * ErrorCode);

  void QueryProc(const TValue & Param1, const TValue & Param2, TValue & Param3) noexcept;

protected:
  Fmu * fmu;
  InputHandler * inputHandler; 
  OutputHandler * outputHandler;
  ParameterFile * parameterFile;
  ModelDescriptionFile * modelDescriptionFile;
  ProcTypeParser procTypeParser;
  Log * log;

  std::unordered_map<channelNumber, std::pair<fmi2ValueReference, fmi2DataType>> channelMapping;

};
