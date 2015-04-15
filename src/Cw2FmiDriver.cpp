
/* This is main driver file - implements all necesarry functions requested by the specification 
of driver version 3.0 of ControlWeb version 6.1
*/

//standard libraries
#include <Windows.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <atlstr.h>
#include <direct.h>

//FMI related
#include <fmilib.h>
#include "Cw2FmiDriver.h"

using namespace std;

/* Control web device driver implementation */
DLLEXPORTUNSIGNED Version() {
	DLOG << "Version 3\n";
	return 0x30000;
}

/* Check operation - no functionality performed */
DLLEXPORTCHAR Check(
    unsigned szErrorString,
    char     *ErrorString,
    unsigned CWVersion,
    unsigned MajorVersion,
    unsigned MinorVersion,
    unsigned APIMajorVersion,
    unsigned APIMinorVersion
) {
	DLOG << "Check\n";
	return 1;
}


DLLEXPORTVOID GetDriverInfo(
    unsigned szDriverName,
    char     *DriverName
) 
{
	DLOG << "getdriverinfo\n";
	strcpy_s(DriverName,szDriverName, "CW2FMI");
	//szDriverName = strlen("CW2Dymola");
}

/* store handle by CW specs, not used in other functionality */
HANDLE * driver ;

DLLEXPORTHANDLE MakeDriver() {
	DLOG << "MakeDriver\n";
	if (driver==NULL) { 
		driver = new HANDLE();
	}
	return driver;
}

DLLEXPORTVOID DisposeDriver(
    HANDLE hDriver
) {
	DLOG << "DisposeDriver\n";
	if (hDriver!=NULL) { 
		delete hDriver;
		DisposeSimulator();
	}
}

typedef unsigned (*tDriverCallback) (HANDLE, unsigned, void *);

/* initializes fmu and parses parameter PAR file provided by ControlWeb*/
DLLEXPORTCHAR Init(
    HANDLE          hDriver,
    unsigned        szParamFileName,
    char            *ParamFileName,
    unsigned        szInitMessage,
    char            *InitMessage,
    unsigned        LoginLevel,
    char            RunningFlag,
    HANDLE          CBHandle,
    tDriverCallback DriverCallback
) {
	//DLOG << "Init\n";
	USES_CONVERSION;
	//this reads INI file, expects to get filename of FMU
	//TODO very dangerous -- do not know whether it already exists or no, not support for multiple FMUs?
	//fmuSim_current = (PFmuSimType)malloc(sizeof(FmuSimType));
	//initialize(fmuSim_current);
	char * fmufilename = new char[1024];
	char * fmutemppath = new char[1024];
	ParseParameters(A2T(ParamFileName),&fmufilename, &fmutemppath);
	if (strlen(fmufilename)==0) {
		strcpy_s(InitMessage,254,"'fmufilename' not defined in PARameter file."); 
		return 0;
	}
	if (strlen(fmutemppath)==0) {
		strcpy_s(InitMessage,254,"'fmutemppath' not defined in PARameter file."); 
		return 0;
	}
	//sprintf_s(fmuSim_current->lasterrormessage,1024,"");
	InitSimulator(fmufilename,fmutemppath);
	if (strlen(fmuSim_current->lasterrormessage)>0) {strcpy_s(InitMessage,1024,fmuSim_current->lasterrormessage); return 0;}
	InitSimulation();
	if (strlen(fmuSim_current->lasterrormessage)>0) {strcpy_s(InitMessage,1024,fmuSim_current->lasterrormessage); return 0;}
	ParseVariables(A2T(ParamFileName));	
	//TODO read simulator parameters FMU filename from param file
	return 1;
}


DLLEXPORTVOID Done(
    HANDLE hDriver
) {
	DLOG << "Done\n";
	return;
}

/* not implemented */
DLLEXPORTCHAR BufferInfo(
    HANDLE        hDriver,
    unsigned      ChannelNumber,
    unsigned char BType,
    unsigned long BLen
) 
{
	DLOG << "BufferInfo\n";
    return false;
}

/* not implemented */
DLLEXPORTVOID SetBufferAddr(
    HANDLE   hDriver,
    unsigned ChannelNumber,
    HANDLE   PBuffer
) {
	DLOG << "SetBufferAddr\n";
	return;
}

/* stores variable/parameter for exchange values in an array buffer (1024), probably some dynamics might be implemented in future */
fmi1_real_t inputRequestValues [1024]; //stored retrieved values
fmi1_value_reference_t readValueReference [1024]; //stores reference id
unsigned readValueChannel[1024];
int readindex = 0;
int getindex = 0;
int readcompleted = 0;


DLLEXPORTVOID InputRequest(
    HANDLE   hDriver,
    unsigned ChannelNumber
) {
	DLOG1 << "InputRequest channel: "<< ChannelNumber << "\n";
	if (ChannelNumber>0) {//readValueReference[readindex++]=0;//ChannelNumber=0 will return current time of simulation
	  readValueChannel[readindex]=ChannelNumber;
	  readValueReference[readindex++]=fmuSim_current->valuesRef[ChannelNumber];
	}
}

/* completes the reading of variables - call of FMU get_real*/
DLLEXPORTVOID InputRequestCompleted(
    HANDLE hDriver
) {
	DLOG1 << "InputRequestCompleted readindex:" << readindex <<" readValueReference[0]:" << readValueReference[0]<<"\n";
	readcompleted = readindex;
	readindex = 0;
	fmi1_import_get_real(fmuSim_current->fmu,readValueReference,readcompleted,inputRequestValues);
	DLOG1 << "InputRequestCompleted inputRequestValues[0]:" << inputRequestValues[0]<<"\n";
	if (fmuSim_current->simulationstarted) StepSimulation();
}

/* finalized the reading of variables */
DLLEXPORTCHAR InputFinalized(
    HANDLE   hDriver,
    unsigned ChannelNumber,
    unsigned *ErrorCode
) {
	DLOG1 << "InputFinalized for " << ChannelNumber << "\n";
	*ErrorCode = 0;
	getindex=0;
	return true;
}

//finds index of referenced variable
char getIndex(unsigned reference) {
	if (readValueChannel[getindex] == reference) {getindex++; return (getindex-1);};
	for (char i=0;i<readcompleted;i++) {
		if (readValueChannel[i]==reference) return i;
	}
	DLOG << "WARNING: Not found value reference in the readValueReference array." << reference <<"\n";
	//returns first index by default
	return 0;
}

DLLEXPORTVOID GetInput(
    HANDLE   hDriver,
    unsigned ChannelNumber,
    TValue   *ChannelValue
)
{
	DLOG1 << "GetInput ";
	switch (ChannelValue->Type) {
	case vtReal:
		ChannelValue->ValReal = ChannelNumber>0?inputRequestValues[getIndex(ChannelNumber)]:fmuSim_current->tcur;//ChannelNumber=0 will return current time of simulation
		DLOG1 << "real value " << ChannelValue->ValReal << "\n";
		break;
	case vtLongReal:
		ChannelValue->ValLongReal = ChannelNumber>0?inputRequestValues[getIndex(ChannelNumber)]:fmuSim_current->tcur;//ChannelNumber=0 will return current time of simulation
		DLOG1 << "longreal value " << ChannelValue->ValLongReal << "\n";
		break;
	case vtPString:
		if (ChannelValue->ValPString !=NULL) {
			strcpy_s((char *) ChannelValue->ValPString, 255,ReadStringFromCache(ChannelNumber));
		}
		DLOG1 << "string value " << ChannelValue->ValPString << "\n";
		break;
	default:
		break;
	}
}

unsigned writeValueReference [256];
char writeindex = 0;
char writecompleted = 0;


/* sets values of variables to model - for parameters it is possible during simulation, for other variables it might be possible before simulation starts */
DLLEXPORTVOID OutputRequest(
    HANDLE   hDriver,
    unsigned ChannelNumber,
    TValue   ChannelValue
) {
	DLOG << "OutputRequest " << ChannelNumber << "\n";
	switch (ChannelValue.Type) {
	case vtReal:
		WriteRealToFMU(ChannelNumber, ChannelValue.ValReal);
		break;
	case vtLongReal:
		WriteDoubleToFMU(ChannelNumber, ChannelValue.ValLongReal);
			break;
	case vtPString:
		if (ChannelValue.PString !=NULL) 
			WriteStringToFMU(ChannelNumber,ChannelValue.PString);
		break;
	default:
		break;
	}
}


DLLEXPORTVOID OutputRequestCompleted(
    HANDLE hDriver
) {
	DLOG << "OutputRequestCompleted\n";
	return;
}


DLLEXPORTCHAR OutputFinalized(
    HANDLE   hDriver,
    unsigned ChannelNumber,
    unsigned *ErrorCode
) {
	DLOG << "OutputFinalized\n";
	*ErrorCode = 0;
	return true;
}



DLLEXPORTVOID DriverProc(
    HANDLE   hDriver,
    unsigned Func,
    unsigned Param1,
    unsigned Param2,
    unsigned Param3,
    unsigned Param4
) {
	DLOG << "DriverProc\n";
	return;
}

/* Query to control simulation
if type of Param1 is string
start,resume - start simulation (default delta)
pause - stop simulation
init - set initial condition to simulation - all variable changes are overwritten


if type of Param1 is real
sets the new delta time in seconds - used for simulation step - between (0.0000 and 1.000 ) e.g. 0.01 

if type of Param1 is integer
number of steps - StepSimulation() is executed within this call
*/
DLLEXPORTVOID QueryProc(
    HANDLE hDriver,
    TValue Param1,
    TValue *Param2
) {
    //USES_CONVERSION;
	DLOG << "QueryProc, paramtype " << Param1.Type << " c"<< Param1.ValCardinal << " lc" << Param1.ValLongCard << " li" << Param1.ValLongInt << " lr" << Param1.ValLongReal << " ps" << Param1.ValPString  << " r" << Param1.ValReal << " sc" <<Param1.ValShortCard << " si" << int(Param1.ValShortInt) <<"\n";
	if (Param1.Type == vtInteger) { //number of steps to be done (default 1), if 0 then start simulation from beginning
		DLOG << "    Performing " << Param1.ValInteger << " simulation steps \n";
		if (Param1.ValInteger == 0) {
		} else 
		for (int i=0;i<Param1.ValInteger;i++) StepSimulation();
	}
	if (Param1.Type == vtLongReal) { //change delta
		DLOG << "    Changing delta to  " << Param1.ValLongReal << "\n";
		fmuSim_current->delta = Param1.ValLongReal;
	}
	if (Param1.Type == vtPString) {
		if (strcmp((char *)Param1.ValPString,"pause" )==0) PauseSimulation();
		else if (strcmp( (char *)Param1.ValPString, "stop")==0) StopSimulation();
		else if (strcmp( (char *)Param1.ValPString, "resume")==0) ResumeSimulation();
		else if (strcmp( (char *)Param1.ValPString, "start")==0) StartSimulation();
		
		else if (strcmp( (char *)Param1.ValPString, "init")==0) ResetSimulationTimes(fmuSim_current->tstart,fmuSim_current->hstep,fmuSim_current->tend);
		else if (strcmp( (char *)Param1.ValPString, "starttime")==0) fmuSim_current->tstart = Param2->ValLongReal;
		else if (strcmp( (char *)Param1.ValPString, "stoptime")==0) fmuSim_current->tend = Param2->ValLongReal;
		else if (strcmp( (char *)Param1.ValPString, "steptime")==0) fmuSim_current->hstep = Param2->ValLongReal;
		
		else if (strcmp( (char *)Param1.ValPString, "continue")==0) ContinueSimulation();		
	}	
}
