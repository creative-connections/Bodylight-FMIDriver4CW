#pragma once

#include "libs/qtronic/XmlParser.h"
#include "libs/qtronic/XmlElement.h"

#include <unordered_map>
#include <tuple>

class ModelDescriptionFile
{
public:
  ModelDescriptionFile(const std::wstring & fmuDir);
  ~ModelDescriptionFile();

  qtronic::ModelDescription * modelDescription;

  std::string getGuid();
  std::string getModelName();
  std::string getModelIdentifier();
  std::string getInstanceName();

  std::unordered_map<channelNumber, std::pair<fmi2ValueReference, fmi2DataType>> determineChannelMapping(const std::unordered_map<channelNumber, std::string> & channels);

protected:
  qtronic::ScalarVariable* getScalarVariableByName(const std::string & name);

};

