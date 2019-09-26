#include "stdafx.h"
#include "Driver.hpp"
#include "Extractor.hpp"

#include <vector>

Driver::~Driver()
{
  // TODO: recursively remove fmuDirectory
  delete(fmu);
  delete(inputHandler);
  delete(log);
  delete(parameterFile);
}

char Driver::Init(unsigned szParamFileName, char * ParamFileName, unsigned szInitMessage, char * InitMessage, unsigned LoginLevel, char RunningFlag, HANDLE CBHandle, tDriverCallback DriverCallback)
{
  parameterFile = new ParameterFile(szParamFileName, ParamFileName);

  log = new Log();
  log->configureDriverLogger(
    parameterFile->driverLoggingOn,
    parameterFile->driverLogDirectory,
    parameterFile->driverLogFileName,
    parameterFile->driverLogVerbosity
  );

  DEBUG("Instantiating a new driver.")
  parameterFile->setLog(log);
  
  // fail on missing .fmu file
  if (!Utils::fs::isFile(parameterFile->fmu)) {
    std::wstring msg = L"FMU file '" + parameterFile->fmu + L"' does not exist.";
    log->error(msg);
    throw wruntime_error(msg);
  }

  std::wstring fmuDir = parameterFile->fmuDestinationDirectory;
  // extract .fmu file to configured directory (fails silently)
  DEBUG(L"Extracting '" + parameterFile->fmu + L"' to '" + fmuDir + L"'.");
  Extractor::extractAll(fmuDir, parameterFile->fmu);

  // load modelDescription.xml (throws on IO error)
  DEBUG(L"Loading modelDescription.xml file.")
  modelDescriptionFile = new ModelDescriptionFile(fmuDir);

  // init FMU with fmu\binaries\{win64|win32}\{model_name}.dll (throws on not found)
  fmu = new Fmu(fmuDir, modelDescriptionFile, log);
  if (parameterFile->instanceName != "") {
    fmu->setInstanceName(parameterFile->instanceName);
  }
  fmu->setVisible(parameterFile->visible);
  fmu->setResourceLocation(parameterFile->resourceLocation);
  fmu->setLogging(parameterFile->fmuLoggingOn);
  fmu->configurationFinished();

  log->configureFmuLogger(
    parameterFile->fmuLoggingOn,
    parameterFile->fmuLogDirectory,
    parameterFile->fmuLogFileName,
    fmu->getInstanceName()
  );
  

  channelMapping = modelDescriptionFile->determineChannelMapping(parameterFile->channels);

  inputHandler = new InputHandler(fmu, log, channelMapping, CBHandle, DriverCallback);
  outputHandler = new OutputHandler(fmu, log, channelMapping, CBHandle, DriverCallback);

  DEBUG(L"Driver sucessfully initialized");
  return 1;
}

void Driver::InputRequest(const channelNumber & ChannelNumber)
{
  inputHandler->InputRequest(ChannelNumber);
}

void Driver::InputRequestCompleted()
{
  inputHandler->InputRequestCompleted();
}

char Driver::InputFinalized(const channelNumber & ChannelNumber, unsigned * ErrorCode)
{
  return inputHandler->InputFinalized(ChannelNumber, ErrorCode);
}

void Driver::GetInput(const channelNumber & ChannelNumber, TValue * ChannelValue)
{
  inputHandler->GetInput(ChannelNumber, ChannelValue);
}

void Driver::OutputRequest(const channelNumber & ChannelNumber, const TValue & ChannelValue)
{
  outputHandler->OutputRequest(ChannelNumber, ChannelValue);
}

void Driver::OutputRequestCompleted()
{
  outputHandler->OutputRequestCompleted();
}

char Driver::OutputFinalized(const channelNumber & ChannelNumber, unsigned * ErrorCode)
{
  return outputHandler->OutputFinalized(ChannelNumber, ErrorCode);
}

/**
  \note 
  Param3 and Param2 might be the same reference, ensure to read from Param2 first
  and only write to Param3 a the end.
*/
void Driver::QueryProc(const TValue & Param1, const TValue & Param2, TValue & Param3) noexcept
{
  // TODO: possibly prioritize for doStep invokation optimization

  // first parameter is not string
  if (Param1.Type != vtDriverString) {
    // TODO: logger and error handling
    Param3.Type = vtError;
    return;
  }

  ProcType type = procTypeParser.parse(static_cast<char*>(Param1.PString), Param1.StringCharLength);

  switch (type) {
  case doStep:
    outputHandler->flush();
    fmu->doStep(Param2);
    break;
  case getLastStepStatus:
    fmu->getLastStepStatus(Param3);
    break;
  case setInstanceName:
    fmu->setInstanceName(Param2);
    break;
  case setVisible:
    TRACE("setVisible called");
    fmu->setVisible(Param2);
    break;
  case setResourceLocation:
    TRACE("setResourceLocation called");
    fmu->setResourceLocation(Param2);
    break;
  case setTolerance:
    TRACE("setTolerance called");
    fmu->setTolerance(Param2);
    break;
  case setStartTime:
    TRACE("setStartTime called");
    fmu->setStartTime(Param2);
    break;
  case initialize:
    TRACE("Instantiating FMU");
    fmu->instantiate();

    TRACE("Entering initialization mode");
    fmu->enterInitializationMode();
    
    TRACE("Loading initial values")
    for (auto it : parameterFile->getInitialValues()) {
      outputHandler->OutputRequest(it.first, it.second);
    }
    outputHandler->OutputRequestCompleted();
    outputHandler->flush(false);

    TRACE("Exiting initialization mode");
    fmu->exitInitializationMode();
    break;
  case setFmuLoggingOn:
    log->configureFmuLogger(
      true,
      parameterFile->fmuLogDirectory,
      parameterFile->fmuLogFileName,
      fmu->getInstanceName()
    );
    fmu->setFmuLoggingOn();
    break;
  case getTime:
    TRACE("getTime called");
    fmu->getTime(Param3);
    break;
  case terminateFmu:
    fmu->terminate();
    break;
  case resetFmu:
    fmu->reset();
    break;
  }
}
