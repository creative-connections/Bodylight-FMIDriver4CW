#include "stdafx.h"
#include "ProcTypeParser.h"

ProcType ProcTypeParser::parse(const char * type)
{
  if (type == nullptr) {
    return ProcType::unknown;
  }
  return procTypeMap[type];
}

ProcType ProcTypeParser::parse(const char * type, size_t size)
{
  if (type == nullptr) {
    return ProcType::unknown;
  }
  return procTypeMap[std::string(type, size)];
}

