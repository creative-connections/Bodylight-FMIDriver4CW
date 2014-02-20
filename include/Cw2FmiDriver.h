// Cw2FmiDriver.h
#ifndef _CWDRIVER_
#define _CWDRIVER_

#ifdef _MSC_VER
#if _MSC_VER >= 1000
#pragma once
#endif
#endif

#include<fmilib.h>

#define DLLEXPORT __declspec( dllexport )
#define DllIMPORT  __declspec(dllimport)
#define DLLEXPORTVOID DLLEXPORT void __cdecl
#define DLLEXPORTDOUBLEARRAY DLLEXPORT double * __cdecl
#define DLLEXPORTCHAR DLLEXPORT char __cdecl
#define DLLEXPORTHANDLE DLLEXPORT HANDLE __cdecl
#define DLLEXPORTUNSIGNED DLLEXPORT unsigned __cdecl
#define DLLEXPORTINT DLLEXPORT int __cdecl
#define MAX_CHANNELS 8192
#define INPUT_CHANNEL_NUMBER 1000
#define OUTPUT_CHANNEL_NUMBER 0
#define PARAMETER_CHANNEL_NUMBER 2000
#define SIZE_OF_TCHAR 1024
#define CHANNELS_SEPARATOR 1000 //defines step by which the channels of variables are separated by type


//////////////////////////////////////////////////////////////////////
// Control Web Internal structures
//////////////////////////////////////////////////////////////////////

#ifdef	_MSC_VER
#pragma pack(push,1)  // set packing, assumed byte alignment
#endif


// values
#define vtNothing          0   // indicates "not having been used before"
#define vtError            1   // error tag
#define vtBoolean          2   // 8 bit flag
#define vtShortCard        3   // 8 bit unsigned
#define vtCardinal         4   // 16 bit unsigned
#define vtLongCard         5   // 32 bit unsigned
#define vtShortInt         6   // 8 bit signed
#define vtInteger          7   // 16 bit signed
#define vtLongInt          8   // 32 bit signed
#define vtReal             9   // float  (dword)
#define vtLongReal         10  // double (qword)
#define vtPString          11  // char (* pString)[255]
#define vtBuffer           12  // buffer
#define vtUnknown          13  // unknown value
#define vtDriverString     15  // driver string

// buffer Header Kinds
#define hkStdBuffer        0   // standard buffer format (header+data block)

// buffer status - bits        // be aware - these value represents bits, not masks
#define stEmpty            0   // empty data
#define stFillInProgress   1   // filling buffer
#define stFull             2   // buffer completely valid
#define stError            3   // not completely valid or if something bad occured
#define stBusy             4   // if setting channel or something calculating
#define stInvalid          5   // 2nd phase after stNonactual (before channels read)
#define stNonactual        6   // if exists new (not actually read) data
#define stModified         15  // used by instrument to mark modification;
// thus can be set channel only if neccessary

struct tBufferHeader { // Buffer header
	struct {
		unsigned short HeaderKind;
		union {
			struct {              // in case of HeaderKind == hkStdBuffer
				unsigned short Channels;     // number of channels
				unsigned       NumSamples;   // volume of data block
				unsigned       CurSamples;   // number of samples currently present
				double         SampFreq;     // sampling frequency or 0 (0 indicates "Time" usage)
				unsigned short SampStruct;   // sample structure (0==real data, 1==complex & Channels==2)
				unsigned short DataType;     // type of data (valueXXX)
				unsigned short Status;       // buffer status (stXXX)
				unsigned short Any1;         // not used now; leave 0
				float          Time;         // sampling time, not period
			};
		};
	};
};                                // size of tBufferHeader must be 32 bytes

typedef char tString255[255];

struct TValue {  // Value storage
	struct {
		unsigned Type;
		union {                                           // switch (Type)
			char               ValBoolean;                // vtBoolean
			unsigned char      ValShortCard;              // vtShortCard
			unsigned short     ValCardinal;               // vtCardinal
			unsigned           ValLongCard;               // vtLongCard
			signed char        ValShortInt;               // vtShortInt
			signed short       ValInteger;                // vtInteger
			signed             ValLongInt;                // vtLongInt
			float              ValReal;                   // vtReal
			double             ValLongReal;               // vtLongReal
			tString255         *ValPString;               // vtPString
			struct {                                      // vtDriverString
				unsigned       StringCharLength;
				void           *PString;
			};
			struct {                                      // vtBuffer
				unsigned char  BType;
				unsigned short BLen256;
				void           *PBuffer;
			};
		};
	};
};
typedef struct FmuSim {
	double delta;
	bool simulationstarted;// = false;
	HINSTANCE hInst;//;
	fmi1_real_t tstart;//=0;
	fmi1_real_t tcur;//=0;// = tstart;
	fmi1_real_t hstep;// = 10000;
	fmi1_real_t tend;// = 2419200.0; //28 days for Hummod 
	fmi1_boolean_t newStep;// = fmi1_true;
	fmi1_value_reference_t valuesRef[128000];

	fmi1_string_t instanceName;// = "CW2FMITest";
	fmi1_string_t fmuGUID;
	fmi1_string_t fmuLocation;// = "";
	fmi1_string_t mimeType;// = "";
	fmi1_real_t timeout;// = 0.0;
	fmi1_boolean_t visible;// = fmi1_false;
	fmi1_boolean_t interactive;// = fmi1_false;


	char * fmufilename;// = new char[1024];
	char * fmutemppath;// = new char [1024];
	char * lasterrormessage;// = new char[1024];
	fmi1_import_t* fmu;	
	fmi_import_context_t* context;
	fmi1_status_t fmistatus;
	jm_status_enu_t jmstatus;
	fmi1_callback_functions_t callBackFunctions;
	jm_callbacks callbacks;
	fmi_version_enu_t version;
	jm_status_enu_t status;
	char * global_tmp_path;
	fmi1_boolean_t StopTimeDefined;// = fmi1_false;
} FmuSimType;

typedef FmuSimType * PFmuSimType;

typedef unsigned (*tDriverCallback) (HANDLE, unsigned, void *);
#ifdef __cplusplus
extern "C" {
#endif
	extern int fmuSim_length;// = 0; //number of instances
	extern PFmuSimType fmuSim_array [1024]; //holder - array of pointers to FmuSim
	extern PFmuSimType fmuSim_current; //pointer to current instance
	extern int fmuSim_currentindex;
	/*extern double delta;
	
	extern HINSTANCE hInst;
	extern fmi1_real_t tstart;//=0;
	extern fmi1_real_t tcur;// = tstart;
	extern fmi1_real_t hstep;// = 10000;
	extern fmi1_real_t tend;// = 2419200.0; //28 days for Hummod 
	extern fmi1_boolean_t newStep;// = fmi1_true;

	extern fmi1_value_reference_t valuesRef[];

	extern char * fmufilename;// = new char[1024];
	extern char * fmutemppath;// = new char [1024];
	extern char * lasterrormessage;
	extern fmi1_import_t* fmu;	
	extern fmi_import_context_t* context;
	extern fmi1_status_t fmistatus;
	extern jm_status_enu_t jmstatus;
	extern bool simulationstarted;
	*/
	

	//fmi1_real_t valuesArr[1024000];

	// functions required by Control Web
	DLLEXPORTUNSIGNED Version();
	DLLEXPORTCHAR Check(unsigned szErrorString, char *ErrorString, unsigned CWVersion, unsigned MajorVersion, unsigned MinorVersion, unsigned APIMajorVersion, unsigned APIMinorVersion);
	DLLEXPORTVOID GetDriverInfo(unsigned szDriverName, char *DriverName);
	DLLEXPORTHANDLE MakeDriver();
	DLLEXPORTVOID DisposeDriver(HANDLE hDriver);
	DLLEXPORTCHAR Init(HANDLE hDriver, unsigned szParamFileName, char *ParamFileName, unsigned szInitMessage, char *InitMessage, unsigned LoginLevel, char RunningFlag, HANDLE CBHandle, tDriverCallback DriverCallback);
	DLLEXPORTCHAR BufferInfo(HANDLE hDriver, unsigned ChannelNumber, unsigned char BType, unsigned long BLen);
	DLLEXPORTVOID InputRequest(HANDLE hDriver, unsigned ChannelNumber);
	DLLEXPORTVOID InputRequestCompleted(HANDLE hDriver);
	DLLEXPORTCHAR InputFinalized(HANDLE hDriver, unsigned ChannelNumber, unsigned *ErrorCode);
	DLLEXPORTVOID GetInput(HANDLE hDriver, unsigned ChannelNumber, TValue *ChannelValue);
	DLLEXPORTVOID OutputRequest(HANDLE hDriver, unsigned ChannelNumber, TValue ChannelValue);
	DLLEXPORTVOID OutputRequestCompleted(HANDLE hDriver);
	DLLEXPORTCHAR OutputFinalized(HANDLE hDriver, unsigned ChannelNumber, unsigned *ErrorCode);
	DLLEXPORTVOID QueryProc(HANDLE hDriver, TValue Param1, TValue *Param2);
	DLLEXPORTVOID SetBufferAddr(HANDLE hDriver,unsigned ChannelNumber, HANDLE   PBuffer);
	DLLEXPORTVOID DriverProc(HANDLE hDriver,unsigned Func,unsigned Param1,unsigned Param2,unsigned Param3,unsigned Param4);
	DLLEXPORTVOID Done(HANDLE hDriver);


	//cw2fmifile


	int GetChannelNumber(LPTSTR channels, int& index);
	DLLEXPORTVOID CreateParameterFile(LPTSTR filename,LPTSTR local_fmu_file, LPTSTR local_fmu_temp);
	DLLEXPORTVOID ParseParameters(LPTSTR filename);
	DLLEXPORTVOID ParseVariables(LPTSTR filename);
	DLLEXPORTVOID CreateDMFFile(LPTSTR filename);
	void GetChannelVariableName(LPTSTR channels, int& index, LPTSTR variableName);
	void AddDefinedChannel (fmi1_value_reference_t myHandle, int channelNumber);

	//cw2fmisimulation


	//specific functions of a driver

	DLLEXPORTVOID DisposeSimulator();
	DLLEXPORTVOID DisposeAllSimulator();
	DLLEXPORTVOID StepSimulation();
	DLLEXPORTVOID StepZeroSimulation(); 
	DLLEXPORTVOID StartSimulation();
	DLLEXPORTVOID StopSimulation();
	DLLEXPORTVOID InitSimulation(); 
	DLLEXPORTVOID InitializeSlave(); 
	DLLEXPORTVOID RestartSimulationIfNeeded();
	DLLEXPORTVOID PauseSimulation();
	DLLEXPORTVOID ResumeSimulation();
	DLLEXPORTVOID ContinueSimulation();
	DLLEXPORTINT InitSimulator(const char * FMUPath, const char * tmpPath);
	DLLEXPORTVOID ReInitSimulatorByName(const char * FMUPath);
	DLLEXPORTVOID ReInitSimulator(int index);
	DLLEXPORTVOID GetVariableValue(char * variableName, double * value);
	DLLEXPORTVOID GetVariableValues(const char ** variableNames, long variableNamesLength,double * values);
	DLLEXPORTVOID GetVariableValuesTest(double * value,long length);
	DLLEXPORTVOID SetVariableValue(char * variableName, double value);
	DLLEXPORTVOID SetStepTime(double value);
	DLLEXPORTVOID ResetSimulationTimes(double start, double step, double end);


	//cw2fmivariables
	fmi1_real_t GetValueFromFMU(int channelindex); 
	//void GetValueFromFMU(int channelindex);
	double ReadDoubleFromCache(int channelindex);
	char *ReadStringFromCache(int channelindex);
	void WriteRealToFMU(int channelindex, float value);
	void WriteDoubleToDymola(int channelindex, double value);
	void WriteStringToDymola(int channelindex, void *value);
	//int GetChannelNumber(char * channels, int &index);
	//void GetChannelVariableName(char * channels, int &index, char *variableName);
	void CleanChannelDefined();

	void WriteDoubleToFMU( unsigned ChannelNumber, double ValLongReal );
	fmi1_value_reference_t GetVariableReference(char * variableName);
	void WriteStringToFMU(int channelindex, void * value);



#define DEBUG
#define DEBUG1
#define DEBUG2

	using namespace std;
	extern ofstream logFile;

#ifdef DEBUG
#define DLOG if(true) logFile 
#else
#define DLOG if(false) logfile
#endif

#ifdef DEBUG1
#define DLOG1 if(true) logFile
#else
#define DLOG1 if(false) logFile
#endif

#ifdef DEBUG2
#define DLOG2 if(true) logFile
#else
#define DLOG2 if(false) logFile
#endif
	
#ifdef __cplusplus
}
#endif
#ifdef	_MSC_VER
#pragma pack(pop)
#endif

#endif // _CWDRIVER_