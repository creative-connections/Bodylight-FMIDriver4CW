//standard libraries
#include <Windows.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <atlstr.h>
#include <direct.h>

#include "Cw2FmiDriver.h"

//#include "StdAfx.h"

using namespace std;

char variableNumber [255];
int GetChannelNumber(LPTSTR channels, long& index) {
	long variableIndex = 0;
	//copy char by char till the ending 0
	while (channels[index]!='=') {
		variableNumber[variableIndex++]= channels[index ++];
	}
	// at the end add the ending 0
	variableNumber[variableIndex]= '\0';
	index++; // move index after the '='
	return atoi(variableNumber);
}

TCHAR variableName2[SIZE_OF_TCHAR];
char variableName3[SIZE_OF_TCHAR];
LPTSTR tempbeginNameIndex;
LPTSTR tempendNameIndex;
static inline int GetChannelNumber2(LPTSTR token) {
	//long variableIndex = 0;
	//copy char by char till the ending 0
	tempendNameIndex = _tcschr(token,'=');
	_tcsncpy_s(variableName2,SIZE_OF_TCHAR,token, tempendNameIndex-token); 
	_tcscat_s(variableName2,SIZE_OF_TCHAR,_T("\0"));  //strncpy doesn't append the zero at the end
	return _tstoi(variableName2);
}
/* reads row with channel description and parses the variable name
it will also move the reading index to the position at the end
INPUT e.g.:  1 = real input 'bloodProperties.pO2'
OUTPUT:bloodProperties.pO2
*/
TCHAR tempvariableName[SIZE_OF_TCHAR] ;
void GetChannelVariableName(LPTSTR channels, long& index, LPTSTR variableName) {
	long variableIndex = 0;
	LPTSTR beginNameIndex;
	LPTSTR endNameIndex;
	USES_CONVERSION;
	//copy char by char till the ending 0
	_tcscpy_s(tempvariableName,SIZE_OF_TCHAR,channels+index);
	index+=_tcslen(tempvariableName);
	
	// gets index of first '
	beginNameIndex = _tcschr(tempvariableName,'\'');
	
	// gets index of second '
	endNameIndex = _tcschr(beginNameIndex+1,'\'');
	
	// what is between ' ' is the variable name
	if ((endNameIndex-beginNameIndex-1) > 0 ) {
		_tcsncpy_s(variableName,SIZE_OF_TCHAR,beginNameIndex+1, endNameIndex-beginNameIndex-1); 
		_tcscat_s(variableName,SIZE_OF_TCHAR,_T("\0"));  //strncpy doesn't append the zero at the end
	}
	else 
		_tcscpy_s(variableName,SIZE_OF_TCHAR,_T("")); //unidentified name - set default 'x'
//	delete tempvariableName;
}

//TCHAR tempvariableName[10] ;
static inline int GetChannelVariableName2(LPTSTR token){
	USES_CONVERSION;
	//copy char by char till the ending 0
	//_tcscpy_s(tempvariableName,SIZE_OF_TCHAR);
	//index+=_tcslen(tempvariableName);

	// gets index of first '
	tempbeginNameIndex = _tcschr(token,'\'');

	// gets index of second '
	tempendNameIndex = _tcschr(tempbeginNameIndex+1,'\'');

	// what is between ' ' is the variable name
	if ((tempendNameIndex-tempbeginNameIndex-1) > 0 ) {
		//_tstoi(beginNameIndex,)
		_tcsncpy_s(variableName2,SIZE_OF_TCHAR,tempbeginNameIndex+1, tempendNameIndex-tempbeginNameIndex-1); 
		_tcscat_s(variableName2,SIZE_OF_TCHAR,_T("\0"));  //strncpy doesn't append the zero at the end
		strcpy_s(variableName3,SIZE_OF_TCHAR,T2A(variableName2));
		return 0;
	}
	else 
		return 1;//_tcscpy_s(variableName,SIZE_OF_TCHAR,_T("")); //unidentified name - set default 'x'
	//	delete tempvariableName;
}



/* sets the number of defined channels to 0 */
void CleanChannelDefined() {
	//iDefinedChannel=0;
}

/* adds a knowledge that a channel was defined with specific index */
void AddDefinedChannel (fmi1_value_reference_t myHandle, int channelNumber) {
	/*DefinedChannel[iDefinedChannel].handle = myHandle;
	DefinedChannel[iDefinedChannel].channelNumber = channelNumber;
	iDefinedChannel++;*/

}

/* returns true if the channelNumber exists in DefinedChannel,
so whether the channelNumber was added in some previous call of AddDefinedChannel */
bool isChannelDefined (int channelNumber) {
	/*for (int i=0;i<iDefinedChannel;i++) {
		if ( DefinedChannel[i].channelNumber == channelNumber) return true;
	}*/
	return false;
}

/* returns true if the variable Name in the myHandle exists in DefinedChannel,
so whether the variable identified by it's name was added in some previous call of AddDefinedChannel */
bool isChannelDefinedForH (fmi1_value_reference_t myHandle) {
	/*for (int i=0;i<iDefinedChannel;i++) {
		if (DefinedChannel[i].handle == myHandle) return true;
	}*/
	return false;
}

/* returns channelnumber under which is the variable defined (from DefinedChannel) */
int getChannelForH(fmi1_value_reference_t myHandle) {
	/*for (int i=0;i<iDefinedChannel;i++) {
		if (DefinedChannel[i].handle == myHandle)
			return DefinedChannel[i].channelNumber;
	}*/
	return 0;
}

static inline LPTSTR NextToken( LPTSTR pArg )
{
	// find next null with strchr and
	// point to the char beyond that.
	return _tcschr( pArg, '\0' ) + 1;
}

//reads token from string "key=value" and coppies the value into the char * variable
void getToken(char * token, char * param1, char ** variable ) 
{
	char * pch;	
	pch = strtok (token,"=");
	DLOG <<"ParsePArameters() getToken() token:"<< token <<"param1:"<< param1;
	if (strcmp(pch,param1)== 0)
	{		
		strcpy_s(*variable,1024,strtok (NULL, "="));
	}
}

/* Parses parameter files and fills the DefinedChannel structure */
DLLEXPORTVOID ParseParameters(LPTSTR filename, char **fmufilename,char ** fmutemppath) {
	LPTSTR channels = new TCHAR[SIZE_OF_TCHAR*1024];
	CleanChannelDefined();
	GetPrivateProfileSection(_T("fmu"),channels,SIZE_OF_TCHAR*1024,filename);
	//parse channels - string delimited by 0 character, last string delimited by second 0 character.
	// Example in the context of GetPrivateProfileSection...
	// Call GetPrivateProfileSection to populate the buffer.
	USES_CONVERSION;
	for ( LPTSTR pToken = channels; pToken && *pToken; pToken = NextToken(pToken) )
	{
		//cout << pToken; // Do something with string
		getToken(T2A(pToken),"filename",fmufilename);
		getToken(T2A(pToken),"temppath",fmutemppath);
		//TODO parse simulation start time end time and step time;
	}
	DLOG.flush();
}

DLLEXPORTVOID ParseVariables(LPTSTR filename) {
	LPTSTR channels = new TCHAR[SIZE_OF_TCHAR*10240];
	//LPTSTR 
	LPTSTR variableName = new TCHAR [SIZE_OF_TCHAR];
	GetPrivateProfileSection(_T("channels"),channels,SIZE_OF_TCHAR*10240,filename);
	//parse channels - string delimited by 0 character, last string delimited by second 0 character.
	// Example in the context of GetPrivateProfileSection...
	// Call GetPrivateProfileSection to populate the buffer.

	USES_CONVERSION;
	long index=0;
	int channelNumber=0;
	LPTSTR sNumber = new TCHAR[10];
	fmi1_import_variable_t* found;

	for ( LPTSTR pToken = channels; pToken && *pToken; pToken = NextToken(pToken) )
	{
		//cout << pToken; // Do something with string
		
		channelNumber = GetChannelNumber2(pToken);
		if (channelNumber>0 && GetChannelVariableName2(pToken)==0) { //it is in global variable variableName2
		  found = fmi1_import_get_variable_by_name(fmuSim_current->fmu, variableName3);
		  fmuSim_current->valuesRef[channelNumber]= fmi1_import_get_variable_vr(found);
		  //DLOG << "parsing "<<channelNumber;
		} else {
			if (channelNumber>0) DLOG << "WARNING not found reference in FMU for channel number "<<channelNumber; //warning for channel 0 is OK
		}

	}
	
/*	while (channels[index]!=0) { //second 0		
		//get channel number and variable name from INI file
		channelNumber = GetChannelNumber(channels,index);
		GetChannelVariableName(channels,index,variableName);
		if (channelNumber>0 && _tcslen(variableName)>2) { //for non-time variables get reference values of FMU and for default 'x' with length 1
		  fmi1_import_variable_t* found = fmi1_import_get_variable_by_name(fmu, T2A(variableName));
		  
		  valuesRef[channelNumber]= fmi1_import_get_variable_vr(found);
		} else {
			DLOG << "not found reference for :"<<channelNumber << " " << variableName <<"\n";

		}

		index++; //go after the first 0
		//assign variableName to the handle produced by Dymola

		//if (channelNumber!=
		//DymosimFindVariableByName(simulator,T2A(variableName),&handle[channelNumber]);		
		//AddDefinedChannel(&handle[channelNumber],channelNumber);
	}*/
	delete channels;    
	delete variableName;
}

/* writes header to ControlWeb parameter file
*/
void WriteHeader(ofstream * file, LPTSTR lfmufilename, LPTSTR lfmutemppath){
	USES_CONVERSION;
	* file << "\
[server]\n\
name = FMI\n\
topic = model\n\
monitor = FALSE\n\
driver_crash = FALSE\n\
debug = FALSE\n\
run_service = FALSE\n\
connect_attempts = 3\n\
connect_timeout = 100\n\
reconnect_attempts = 20\n\
reconnect_timeout = 200\n\
operation_timeout = 50\n\
operation_attempts = 3\n\
service_path = \"\"\n\
decimal_char = .\n\
true_text = 1\n\
false_text = 0\n\
item_sep = ,\n\
data_sep = 0D0A00\n\
request_limit = 5\n\
\n\
[fmu]\n\
filename="<< T2A(lfmufilename) <<"\n\
temppath="<< T2A(lfmutemppath) <<"\n\
simulation_start_time = 0\n\
simulation_stop_time = 2419200\n\
simulation_step_time = 10000\n\
\n\
[channels]\n\
0 = real bidirectional, 'time'\n\
";
}

/* Writes a row with channelnumber, variable name and type
*/
void WriteVariableRow(ofstream * file, fmi1_value_reference_t myhandle,const char ** myhandlename, int number) {
	* file << number << " = ";
	* file << " real";
	* file << " bidirectional";
	* file << ", '" << myhandlename << "'\n";
}

int channelnumber= 1;
/* Writes a row with channelnumber, variable name and type
*/
void WriteVariableRow(ofstream * file, fmi1_import_variable_t* var) {

	* file << channelnumber++ << " = ";
	* file << " real";
	* file << " bidirectional";	
	* file << ", '" << fmi1_import_get_variable_name(var) << "'\n";
}

/* structures to keep information about DMF numbers used in ControlWeb DMF files */
int dmfnumber = 0;
int begindmfnumber=0;
char dmfstring[255] = "";

/* returns true if current variable handle is different from previous handles,
so new row is needed in DMF file*/
bool NeedDMFString(fmi1_value_reference_t myhandle, fmi1_value_reference_t lasthandle) {
	/*if (lasthandle==NULL) return false;
	if (((myhandle->basetype) & dsBaseTypeMask) != ((lasthandle->basetype) & dsBaseTypeMask) ) return true;
	if ((myhandle->category) != (lasthandle->category)) return true;*/
	return false;
}

/* returns DMF row based on the variable handle
*/
void NewDMFString(fmi1_value_reference_t myhandle) {
	strcpy_s(dmfstring,255,"");
	/*switch((myhandle->basetype) & dsBaseTypeMask) {
	case  dsReal:*/
		strcat_s(dmfstring,255,"real");
/*		break;
	case  dsBoolean:
		strcat_s(dmfstring,255,"boolean");
		break;
	case  dsInteger:
		strcat_s(dmfstring,255,"integer");
		break;*/
  //dsBaseTypeMask=3,
  //dsConnector
/*  dsNormalStart=8,
  dsContinueStart=16,
  dsOptionalStart=32,
  dsStartMasks=56,
  dsDiscrete=64,
  dsFlow=128,
  dsReserved=256,
  dsStoreMask=512|1024,
  dsStoredPublic=0,
  dsNonStored=512,
  dsProtected=1024,
  dsStoredProtected=512|1024*/
/*	default: strcat_s(dmfstring,"real");
	}

	switch (myhandle->category) {
	case dsParameter: strcat_s(dmfstring," bidirectional");
		break;
	case dsInput: strcat_s(dmfstring," output");
		//TODO advicedata|advicedatanoread
//		* file << " advicedata";
		break;
	case dsOutput: strcat_s(dmfstring," input");
		break;
	default : */
		strcat_s(dmfstring," bidirectional");
	//}
	strcat_s(dmfstring,"\n");
}

/* writes one row to a DMF file */
/*void WriteDMFRow(ofstream * file, DymosimVariableHandle * myhandle, const char ** aliasName, int number) {	
	if (dmfnumber==0) { //zacatek
		* file << "begin\n  " << number;
		dmfnumber = number;
		begindmfnumber = number; // pocatecni cislo intervalu
		NewDMFString(myhandle);
		lastdmfhandle = myhandle;
	} else  
	if (((number-dmfnumber)>1) || NeedDMFString(myhandle, lastdmfhandle)) {//konec sekce, krok o vice nez 1 kanal 
		if (begindmfnumber == dmfnumber) * file << " " << dmfstring; // pocatecni cislo intervalu = koncove cislo intervalu - staci jedno
		else * file << " - " << dmfnumber << " " << dmfstring;
		dmfnumber = number;
		begindmfnumber = number;
		NewDMFString(myhandle);
		lastdmfhandle = myhandle;
		* file << "  " << dmfnumber;
	} else
	{ //neni potreba novy radek
		dmfnumber = number;
	}
}*/

/*creates parameter file - 
based on the previously parsed file by the call ParseParameters()
- new variables will be added 
- existing variables will be preserved in numbers*/
DLLEXPORTVOID CreateParameterFile(LPTSTR filename,LPTSTR local_fmu_file, LPTSTR local_fmu_temp) {
//	int state;
	USES_CONVERSION;
	int inputChannelNumber=INPUT_CHANNEL_NUMBER,outputChannelNumber=OUTPUT_CHANNEL_NUMBER,otherChannelNumber=PARAMETER_CHANNEL_NUMBER;
	ofstream parameterFile;
	DLOG << "creating parameter file\n";
	DLOG.flush();
	parameterFile.open(filename);
	WriteHeader(&parameterFile,local_fmu_file,local_fmu_temp);

	InitSimulator(T2A(local_fmu_file),T2A(local_fmu_temp));
	DLOG.flush();
	parameterFile.flush();
	InitSimulation(); //fix access violation 
	DLOG.flush();

	fmi1_import_variable_list_t* vl = fmi1_import_get_variable_list(fmuSim_current->fmu);
	size_t nv = fmi1_import_get_variable_list_size(vl);
	size_t i;
	//i = (unsigned)nv;
	//assert(i == nv);
	DLOG << "There are " << (unsigned)nv << "variables in total \n";
	for(i = 0; i < nv; i++) {
		fmi1_import_variable_t* var = fmi1_import_get_variable(vl, i);
		if(!var) {
			DLOG << "Something wrong with variable" << i << "\n";
			//do_exit(1);
		}
		else {
			WriteVariableRow(&parameterFile,var);
		}
	}
	fmi1_import_free_variable_list(vl);

	parameterFile.close();
	DisposeSimulator();
}
