#include "stdafx.h"
#include "stdafx.h"

#include "Fmu.hpp"
#include "Logger.hpp"

#include "libs/qtronic/XmlParser.h"
#include "libs/qtronic/XmlElement.h"

/**
 * Encoding: std::strings used in this class must be UTF-8 encoded
 *           since fmi2String used in the FMI2.0 C API is also UTF-8 encoded.
 *           modelDescription.xml parser must return UTF-8 encoded char pointers
 */
Fmu::Fmu(const std::wstring & fmuDir, ModelDescriptionFile * modelDescriptionFile, Log * log)
  :modelDescriptionFile(modelDescriptionFile), log(log)
{
  guid = modelDescriptionFile->getGuid();
  modelName = modelDescriptionFile->getModelName();
  modelIdentifier = modelDescriptionFile->getModelIdentifier();
  instanceName = modelDescriptionFile->getInstanceName();

  loadDll(fmuDir);
}

void Fmu::configurationFinished()
{
  logDescription();
}

fmi2CallbackFunctions Fmu::getCallbackFunctions()
{
  Log & logger = *log;
  const fmi2CallbackFunctions cbf = {
    &logger.fmuLoggingFunction,
    calloc,
    free,
    NULL,
    NULL
  };
  return cbf;
}

std::wstring Fmu::getDllPath(const std::wstring & fmuDir)
{
#ifdef _WIN64
  std::wstring dllDir = L"binaries\\win64\\";
#else
  std::wstring dllDir = L"binaries\\win32\\";
#endif

  return fmuDir + dllDir + Utils::enc::utf8ToWstring(modelIdentifier) + L".dll";
}

void Fmu::loadDll(const std::wstring & fmuDir) 
{
  std::wstring dllPath = getDllPath(fmuDir);

  DEBUG(L"Loading dll file '" + dllPath + L"'.");
  HMODULE h = LoadLibraryW(dllPath.c_str());

  if (!h) {
    std::wstring msg = L"Dll file '" + dllPath + L"' could not be loaded";
    log->error(msg);
    throw wruntime_error(msg);
  }

  fmu.dllHandle = h;
  fmu.getTypesPlatform         = (fmi2GetTypesPlatformTYPE *)         getAdr(h, "fmi2GetTypesPlatform");
  fmu.getVersion               = (fmi2GetVersionTYPE *)               getAdr(h, "fmi2GetVersion");
  fmu.setDebugLogging          = (fmi2SetDebugLoggingTYPE *)          getAdr(h, "fmi2SetDebugLogging");
  fmu.instantiate              = (fmi2InstantiateTYPE *)              getAdr(h, "fmi2Instantiate");
  fmu.freeInstance             = (fmi2FreeInstanceTYPE *)             getAdr(h, "fmi2FreeInstance");
  fmu.setupExperiment          = (fmi2SetupExperimentTYPE *)          getAdr(h, "fmi2SetupExperiment");
  fmu.enterInitializationMode  = (fmi2EnterInitializationModeTYPE *)  getAdr(h, "fmi2EnterInitializationMode");
  fmu.exitInitializationMode   = (fmi2ExitInitializationModeTYPE *)   getAdr(h, "fmi2ExitInitializationMode");
  fmu.terminate                = (fmi2TerminateTYPE *)                getAdr(h, "fmi2Terminate");
  fmu.reset                    = (fmi2ResetTYPE *)                    getAdr(h, "fmi2Reset");
  fmu.getReal                  = (fmi2GetRealTYPE *)                  getAdr(h, "fmi2GetReal");
  fmu.getInteger               = (fmi2GetIntegerTYPE *)               getAdr(h, "fmi2GetInteger");
  fmu.getBoolean               = (fmi2GetBooleanTYPE *)               getAdr(h, "fmi2GetBoolean");
  fmu.getString                = (fmi2GetStringTYPE *)                getAdr(h, "fmi2GetString");
  fmu.setReal                  = (fmi2SetRealTYPE *)                  getAdr(h, "fmi2SetReal");
  fmu.setInteger               = (fmi2SetIntegerTYPE *)               getAdr(h, "fmi2SetInteger");
  fmu.setBoolean               = (fmi2SetBooleanTYPE *)               getAdr(h, "fmi2SetBoolean");
  fmu.setString                = (fmi2SetStringTYPE *)                getAdr(h, "fmi2SetString");
  fmu.getFMUstate              = (fmi2GetFMUstateTYPE *)              getAdr(h, "fmi2GetFMUstate");
  fmu.setFMUstate              = (fmi2SetFMUstateTYPE *)              getAdr(h, "fmi2SetFMUstate");
  fmu.freeFMUstate             = (fmi2FreeFMUstateTYPE *)             getAdr(h, "fmi2FreeFMUstate");
  fmu.serializedFMUstateSize   = (fmi2SerializedFMUstateSizeTYPE *)   getAdr(h, "fmi2SerializedFMUstateSize");
  fmu.serializeFMUstate        = (fmi2SerializeFMUstateTYPE *)        getAdr(h, "fmi2SerializeFMUstate");
  fmu.deSerializeFMUstate      = (fmi2DeSerializeFMUstateTYPE *)      getAdr(h, "fmi2DeSerializeFMUstate");
  fmu.getDirectionalDerivative = (fmi2GetDirectionalDerivativeTYPE *) getAdr(h, "fmi2GetDirectionalDerivative");
  fmu.setRealInputDerivatives  = (fmi2SetRealInputDerivativesTYPE *)  getAdr(h, "fmi2SetRealInputDerivatives");
  fmu.getRealOutputDerivatives = (fmi2GetRealOutputDerivativesTYPE *) getAdr(h, "fmi2GetRealOutputDerivatives");
  fmu.doStep                   = (fmi2DoStepTYPE *)                   getAdr(h, "fmi2DoStep");
  fmu.cancelStep               = (fmi2CancelStepTYPE *)               getAdr(h, "fmi2CancelStep");
  fmu.getStatus                = (fmi2GetStatusTYPE *)                getAdr(h, "fmi2GetStatus");
  fmu.getRealStatus            = (fmi2GetRealStatusTYPE *)            getAdr(h, "fmi2GetRealStatus");
  fmu.getIntegerStatus         = (fmi2GetIntegerStatusTYPE *)         getAdr(h, "fmi2GetIntegerStatus");
  fmu.getBooleanStatus         = (fmi2GetBooleanStatusTYPE *)         getAdr(h, "fmi2GetBooleanStatus");
  fmu.getStringStatus          = (fmi2GetStringStatusTYPE *)          getAdr(h, "fmi2GetStringStatus");
  fmu.loaded = true;
}

void * Fmu::getAdr(HMODULE dllHandle, const char * functionName)
{
    void * fp = GetProcAddress(dllHandle, functionName);

    // TODO: define set of required functions and don't hard fail if some of them are missing
    if (!fp) {
      std::wstring msg = std::wstring(L"Function ") + Utils::enc::acpToWstring(functionName) + L" was not found in the FMU";
      log->error(msg);
      throw wruntime_error(msg);
    }

    TRACE("FMU dll function " + std::string(functionName) + " was loaded.");
    return fp;
}

void Fmu::logDescription()
{
  DEBUG("Model information");
  DEBUG("[GUID            ]: " + guid);
  DEBUG("[Model name      ]: " + modelName);
  DEBUG("[Model identifier]: " + modelIdentifier);
  DEBUG("[Instance name   ]: " + instanceName);
  DEBUG("[visible         ]: " + std::to_string(visible));
  DEBUG("[resourceLocation]: " + resourceLocation);
  DEBUG("[logging         ]: " + std::to_string(logging));
  DEBUG("[startTime       ]: " + std::to_string(startTime));
  DEBUG("[tolerance       ]: " + std::to_string(tolerance));
}

Fmu::~Fmu()
{
  if (fmu.loaded) {
    if (instantiated) {
      fmu.terminate(instance);
    }
    // some implementations of FMU crash on freeInstance(nullptr), against the spec
    if (instance != nullptr) {
      fmu.freeInstance(instance);
    }
    FreeLibrary(fmu.dllHandle);
  }
}

void Fmu::getReal(const std::vector<fmi2ValueReference>& references, std::unordered_map<fmi2ValueReference, fmi2Real>& outputs)
{
  const size_t & size = references.size();
  fmi2Real * values = new fmi2Real[size];

  const fmi2ValueReference * refs = &references.front();
  fmu.getReal(instance, refs, size, values);

  for (int fori = 0; fori < size; fori++) {
    outputs[refs[fori]] = values[fori]; // copy value to output
  }

  delete[] values;
}

void Fmu::getInteger(const std::vector<fmi2ValueReference>& references, std::unordered_map<fmi2ValueReference, fmi2Integer>& outputs)
{
  const size_t & size = references.size();
  fmi2Integer * values = new fmi2Integer[size];

  const fmi2ValueReference * refs = &references.front();
  fmu.getInteger(instance, refs, size, values);

  for (int fori = 0; fori < size; fori++) {
    outputs[refs[fori]] = values[fori]; // copy value to output
  }

  delete[] values;
}

void Fmu::getBoolean(const std::vector<fmi2ValueReference>& references, std::unordered_map<fmi2ValueReference, fmi2Boolean>& outputs)
{
  const size_t & size = references.size();
  fmi2Boolean * values = new fmi2Boolean[size];

  const fmi2ValueReference * refs = &references.front();
  fmu.getBoolean(instance, refs, size, values);

  for (int fori = 0; fori < size; fori++) {
    outputs[refs[fori]] = values[fori]; // copy value to output
  }

  delete[] values;
}

void Fmu::setReal(const std::vector<fmi2ValueReference>& references, const std::vector<fmi2Real>& inputs)
{
  const size_t & size = references.size();
  const fmi2ValueReference * refs = &references.front();
  const fmi2Real * inps = &inputs.front();
  fmu.setReal(instance, refs, size, inps);
  // TODO: check for fmi2Status
}

void Fmu::setInteger(const std::vector<fmi2ValueReference>& references, const std::vector<fmi2Integer>& inputs)
{
  const size_t & size = references.size();
  const fmi2ValueReference * refs = &references.front();
  const fmi2Integer * inps = &inputs.front();
  fmu.setInteger(instance, refs, size, inps);
  // TODO: check for fmi2Status
}

void Fmu::setBoolean(const std::vector<fmi2ValueReference>& references, const std::vector<fmi2Boolean>& inputs)
{
  const size_t & size = references.size();
  const fmi2ValueReference * refs = &references.front();
  const fmi2Boolean * inps = &inputs.front();
  fmu.setBoolean(instance, refs, size, inps);
}

void Fmu::setInstanceName(const TValue & value)
{
  instanceName = Utils::tvalue::getString(value, modelName);
}

void Fmu::setInstanceName(const std::string & value)
{
  instanceName = value;
}

std::string Fmu::getInstanceName()
{
  return instanceName;
}

void Fmu::setResourceLocation(const TValue & value)
{
  resourceLocation = Utils::tvalue::getString(value, "file://");
}

void Fmu::setResourceLocation(const std::string & value)
{
  resourceLocation = value;
}

void Fmu::setVisible(const TValue & value)
{
  visible = Utils::tvalue::getBoolean(value, true);
}

void Fmu::setVisible(const fmi2Boolean & value)
{
  visible = value;
}

void Fmu::setTolerance(const TValue & value)
{
  toleranceDefined = fmi2True;
  tolerance = Utils::tvalue::getReal(value, 0.0001);
}

void Fmu::setTolerance(const fmi2Real & value)
{
  if (value < 0.0) {
    toleranceDefined = fmi2False;
    return;
  }
  tolerance = value;
}

void Fmu::setStartTime(const TValue & value)
{
  startTime = Utils::tvalue::getFmi2Real(value);
}

void Fmu::setStartTime(const fmi2Real & value)
{
  startTime = value;
}

void Fmu::getTime(TValue & value)
{
  value.Type = vtLongReal;
  value.ValLongReal = time;
  TRACE("getTime: " + std::to_string(time));
}

void Fmu::getLastStepStatus(TValue & value)
{
  value.Type = vtInteger;
  value.ValInteger = status;
}

void Fmu::setFmuLoggingOn()
{
  logging = fmi2True;
  fmu.setDebugLogging(instance, fmi2True, 0, NULL);
}

void Fmu::setLogging(const fmi2Boolean & value)
{
  logging = value;
}

void Fmu::instantiate()
{
  if (!instantiated) {
    // TODO: logging load from parameter file
    instance = fmu.instantiate(instanceName.c_str(),
      fmi2CoSimulation,
      guid.c_str(),
      resourceLocation.c_str(),
      &getCallbackFunctions(),
      visible,
      logging);
    instantiated = true;
  }
}

void Fmu::enterInitializationMode()
{
  // TODO: logging categories load from parameter file and use fmu.setDebugLogging()
  // TODO: extend support for stop time and steadystate simulations
  fmu.setupExperiment(
    instance, 
    toleranceDefined, tolerance, 
    startTime,
    stopTimeDefined, stopTime
  );

  time = startTime;
  fmu.enterInitializationMode(instance);
}

void Fmu::exitInitializationMode()
{
  fmu.exitInitializationMode(instance);
}

void Fmu::terminate()
{
  fmu.terminate(instance);
}

void Fmu::reset()
{
  fmu.reset(instance);
}

void Fmu::doStep(const TValue & stepSize)
{
  // TODO: load default step size from parameter file
  fmi2Real step = Utils::tvalue::getReal(stepSize, 0.01);
  TRACE("doStep size: " + std::to_string(step));
  status = fmu.doStep(instance, time, step, fmi2True);
  time += step;
  TRACE("doStep status: " + status);
  TRACE("time is now " + std::to_string(time));
}

