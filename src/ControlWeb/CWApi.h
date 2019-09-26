#pragma once

extern "C" {
  __declspec(dllexport) unsigned __cdecl Version();

  __declspec(dllexport) char Check(
    unsigned szErrorString,
    char     *ErrorString,
    unsigned CWVersion,
    unsigned MajorVersion,
    unsigned MinorVersion,
    unsigned APIMajorVersion,
    unsigned APIMinorVersion
  );

  __declspec(dllexport) void GetDriverInfo(
    unsigned szDriverName,
    char     *DriverName
  );

  __declspec(dllexport) HANDLE MakeDriver();

  __declspec(dllexport) void DisposeDriver(
    HANDLE hDriver
  );

  __declspec(dllexport) char Init(
    HANDLE          hDriver,
    unsigned        szParamFileName,
    char            *ParamFileName,
    unsigned        szInitMessage,
    char            *InitMessage,
    unsigned        LoginLevel,
    char            RunningFlag,
    HANDLE          CBHandle,
    tDriverCallback DriverCallback
  );

  __declspec(dllexport) void Done(
    HANDLE hDriver
  );

  __declspec(dllexport) char BufferInfo(
    HANDLE        hDriver,
    unsigned      ChannelNumber,
    unsigned char BType,
    unsigned long BLen
  );

  __declspec(dllexport) void SetBufferAddr(
    HANDLE   hDriver,
    unsigned ChannelNumber,
    HANDLE   PBuffer
  );

  __declspec(dllexport) void InputRequest(
    HANDLE   hDriver,
    unsigned ChannelNumber
  );

  __declspec(dllexport) void InputRequestCompleted(
    HANDLE hDriver
  );

  __declspec(dllexport) char InputFinalized(
    HANDLE   hDriver,
    unsigned ChannelNumber,
    unsigned *ErrorCode
  );

  __declspec(dllexport) void GetInput(
    HANDLE   hDriver,
    unsigned ChannelNumber,
    TValue   *ChannelValue
  );

  __declspec(dllexport) void GetInput2(
    HANDLE    hDriver,
    unsigned  ChannelNumber,
    TValue    *ChannelValue,
    unsigned  *QOS,
    timeStamp *TimeStamp
  );

  __declspec(dllexport) void GetInput3(
    HANDLE    hDriver,
    unsigned  ChannelNumber,
    TValue    *ChannelValue,
    unsigned  *QOS,
    timeStamp *TimeStamp,
    unsigned  *ErrorCode
  );

  __declspec(dllexport) void OutputRequest(
    HANDLE   hDriver,
    unsigned ChannelNumber,
    TValue   ChannelValue
  );

  __declspec(dllexport) void OutputRequest2(
    HANDLE    hDriver,
    unsigned  ChannelNumber,
    TValue    ChannelValue,
    unsigned  QOS,
    timeStamp *TimeStamp
  );

  __declspec(dllexport) void OutputRequest3(
    HANDLE    hDriver,
    unsigned  ChannelNumber,
    TValue    ChannelValue,
    unsigned  QOS,
    timeStamp *TimeStamp
  );

  __declspec(dllexport) void OutputRequestCompleted(
    HANDLE hDriver
  );

  __declspec(dllexport) char OutputFinalized(
    HANDLE   hDriver,
    unsigned ChannelNumber,
    unsigned *ErrorCode
  );

  __declspec(dllexport) void DriverProc(
    HANDLE   hDriver,
    unsigned Func,
    unsigned Param1,
    unsigned Param2,
    unsigned Param3,
    unsigned Param4
  );

  __declspec(dllexport) void QueryProc(
    HANDLE hDriver,
    TValue Param1,
    TValue *Param2
  );

  __declspec(dllexport) void QueryProc3(
    HANDLE hDriver,
    TValue Param1,
    TValue Param2,
    TValue *Param3
  );

  __declspec(dllexport) char InputOOBDataQuery(
    HANDLE        hDriver,
    unsigned long *State,
    unsigned      *ChannelNumber
  );

  __declspec(dllexport) unsigned DriverCallback(
    HANDLE   CBHandle,
    unsigned Func,
    void     *Param
  );
}