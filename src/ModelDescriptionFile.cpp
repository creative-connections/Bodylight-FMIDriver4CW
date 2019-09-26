#include "stdafx.h"

#include "ModelDescriptionFile.hpp"

#include "libs/qtronic/XmlParser.h"
#include "libs/qtronic/XmlElement.h"

#include "Logger.hpp"

ModelDescriptionFile::ModelDescriptionFile(const std::wstring & fmuDir)
{
  // XmlParser does not support wchar_t for file location
  std::string modelDescriptionPath = Utils::enc::wstringToStringNoConv(fmuDir) + "modelDescription.xml";

  qtronic::XmlParser parser(Utils::str::bufferDup(modelDescriptionPath));
  modelDescription = parser.parse();
}

ModelDescriptionFile::~ModelDescriptionFile()
{
}

std::string ModelDescriptionFile::getGuid()
{
  return modelDescription->getAttributeValue(qtronic::XmlParser::Att::att_guid);
}

std::string ModelDescriptionFile::getModelName()
{
  return modelDescription->getAttributeValue(qtronic::XmlParser::Att::att_modelName);
}

std::string ModelDescriptionFile::getModelIdentifier()
{
  return modelDescription->coSimulation->getAttributeValue(qtronic::XmlParser::Att::att_modelIdentifier);
}

std::string ModelDescriptionFile::getInstanceName()
{
  return getModelName();
}

qtronic::ScalarVariable* ModelDescriptionFile::getScalarVariableByName(const std::string & name) 
{
  std::vector<qtronic::ScalarVariable *> modelVariables = modelDescription->modelVariables;
 
  for (auto it : modelVariables)
  {
    if (it->getAttributeValue(qtronic::XmlParser::att_name) == name) {
      return it;
    }
  }
  throw wruntime_error(L"The variable '" + Utils::enc::utf8ToWstring(name) + L"' was not found in the FMU");
}

std::unordered_map<channelNumber, std::pair<fmi2ValueReference, fmi2DataType>> ModelDescriptionFile::determineChannelMapping(const std::unordered_map<channelNumber, std::string> & channels)
{
  std::unordered_map<channelNumber, std::pair<fmi2ValueReference, fmi2DataType>> out;
  qtronic::ScalarVariable * variable;
  for (const auto &it : channels)
  {
    variable = getScalarVariableByName(it.second);

    fmi2DataType type;
    switch (variable->typeSpec->type) {
    case (qtronic::XmlParser::Elm::elm_Real): type = FMI_REAL; break;
    case (qtronic::XmlParser::Elm::elm_Integer): type = FMI_INTEGER; break;
    case (qtronic::XmlParser::Elm::elm_Boolean): type = FMI_BOOLEAN; break;
    case (qtronic::XmlParser::Elm::elm_String): type = FMI_STRING; break;
    default: // should never happen, fmu is not sane
      throw wruntime_error(L"Unexpected variable type in modelDescription.xml for variable '" + Utils::enc::utf8ToWstring(it.second) + L"'");
      break;
    }

    out[it.first] = std::make_pair(variable->getValueReference(), type);
  }

  return out;
}