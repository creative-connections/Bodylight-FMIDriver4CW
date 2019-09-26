#pragma once

#include "Utils.h"

class wruntime_error : public std::runtime_error
{
public:
  typedef std::runtime_error _Mybase;

  // wchar based 
  explicit wruntime_error(const std::wstring& _Message) : 
    _Mybase(Utils::enc::wstringToAcp(_Message).c_str())
  {
  }

  explicit wruntime_error(const wchar_t *_Message) : 
    _Mybase(Utils::enc::wstringToAcp(std::wstring(_Message)))
  {	
  }
  
};