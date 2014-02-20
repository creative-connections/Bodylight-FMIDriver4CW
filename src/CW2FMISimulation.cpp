//standard libraries
#include <Windows.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <atlstr.h>
#include <direct.h>
#include <ctime>


#include "Cw2FmiDriver.h"
//#include "StdAfx.h"


int fmuSim_length = 0; //number of instances
PFmuSimType fmuSim_array [1024]; //holder - array of pointers to FmuSim
PFmuSimType fmuSim_current; //pointer to current instance
int fmuSim_currentindex = -1;

void initialize(PFmuSimType p){
	p->delta=0;
	p->simulationstarted = false;
	p->tstart=0;
	p->tcur=0;// = tstart;
	p->hstep = 100;
	p->tend = 600; //28 days for Hummod 
	p->newStep = fmi1_true;	
	p->instanceName = "CW2FMITest";	
	p->fmuLocation = "";
	p->mimeType = "";
	p->timeout = 0.0;
	p->visible = fmi1_false;
	p->interactive = fmi1_false;
	p->fmu = NULL;
	p->fmufilename = new char[1024];
	p->fmutemppath = new char [1024];
	p->lasterrormessage = new char[1024];
	p->StopTimeDefined = fmi1_false;
	p->global_tmp_path = new char[1024];
}
/*
*/


using namespace std;

ofstream logFile;


/*	char * fmufilename = new char[1024];
	char * fmutmppath = new char [1024];
	fmi1_import_t* fmu;	
	fmi_import_context_t* context;
*/

/* specific FMI logger callback function for ControlWeb */
void cwlogger(jm_callbacks* c, jm_string module, jm_log_level_enu_t log_level, jm_string message)
{
	DLOG << "module=" << module <<" log level=" << jm_log_level_to_string(log_level) <<":" << message << "\n";

}

/* Initializes simulator, 
for debuging creates log file, 
loads <FMU>.dll and extracts it into temp path / temp paths should be absolute path - otherwise there might be undefined behavior, 

call once in the beginning.*/
DLLEXPORTINT InitSimulator(const char * FMUPath, const char * tmpPath) {

	_mkdir(tmpPath);
	//add fmuinstance
	fmuSim_current = (PFmuSimType) malloc(sizeof(FmuSimType));
	initialize(fmuSim_current); 
	fmuSim_currentindex = fmuSim_length;
	fmuSim_array[fmuSim_length++] = fmuSim_current;
	
	strcpy_s(fmuSim_current->global_tmp_path,1024,tmpPath);

	//needs to be initialized for logging purposes
	char * logfilename = new char[1024]; strcpy_s(logfilename,1024,tmpPath);
	strcat_s(logfilename,1024,"fmi.log");
	//open a log file only if it was not opened before, otherwise the log will be logged into the already opened stream
	if (!logFile.is_open()){
	  DLOG.open(logfilename);
	}
	  DLOG << "Initializing CW2FMI version 1.00 for " << FMUPath << "\n";
	  DLOG.flush();
	

	strcpy_s(fmuSim_current->fmufilename,1024,FMUPath);
	strcpy_s(fmuSim_current->fmutemppath,1024,tmpPath);
	//tmpPath = "tempfmu";//argv[2]; //TODO define path where FMU should be extracted
	//DLOG << "creating temporary directory" << tmpPath <<"\n";

	fmuSim_current->callbacks.malloc = malloc;
	fmuSim_current->callbacks.calloc = calloc;
	fmuSim_current->callbacks.realloc = realloc;
	fmuSim_current->callbacks.free = free;
	fmuSim_current->callbacks.logger = cwlogger;
#ifdef DEBUG1
	fmuSim_current->callbacks.log_level = jm_log_level_debug;//jm_log_level_warning;//jm_log_level_debug;
#else
	fmuSim_current->callbacks.log_level = jm_log_level_warning;//jm_log_level_debug;
#endif
	fmuSim_current->callbacks.context = 0;

	fmuSim_current->callBackFunctions.logger = fmi1_log_forwarding;
	fmuSim_current->callBackFunctions.allocateMemory = calloc;
	fmuSim_current->callBackFunctions.freeMemory = free;

	fmuSim_current->context = fmi_import_allocate_context(&fmuSim_current->callbacks);

	fmuSim_current->version = fmi_import_get_fmi_version(fmuSim_current->context, FMUPath, tmpPath);

	if(fmuSim_current->version != fmi_version_1_enu) {
		DLOG << "Only FMU version 1.0 is supported so far. Your version is" << fmuSim_current->version <<".\n";
	    sprintf_s(fmuSim_current->lasterrormessage,1024,"Only FMU version 1.0 is supported so far. Your version is %d.", fmuSim_current->version);
		return -1;
	}

	fmuSim_current->fmu = fmi1_import_parse_xml(fmuSim_current->context, tmpPath);

	if(!fmuSim_current->fmu) {
		DLOG << "Error parsing XML of FMU.\n";
		return -1;		
	}
	char* buffer;

	// Get the current working directory: 
	if( (buffer = _getcwd( NULL, 0 )) == NULL )
		perror( "_getcwd error" );
	else
	{
		DLOG << "current directory:" << buffer << "\n";
		free(buffer);
	}

	fmuSim_current->status = fmi1_import_create_dllfmu(fmuSim_current->fmu, fmuSim_current->callBackFunctions, 1);
	if (fmuSim_current->status == jm_status_error) {
		DLOG << "Could not create the DLL loading mechanism(C-API) (error: "<< fmi1_import_get_last_error(fmuSim_current->fmu) <<" \n";
		return -1;		
	}
	return fmuSim_currentindex;
	//simulation 	test_simulate_cs(fmu,ref1,ref2);
}
/*
finds already initialized simulator among the fmusim_array and sets current simulation instance to it
*/
DLLEXPORTVOID ReInitSimulatorByName(const char * FMUPath) {
	//if there is only 1 simulator - then fmuSim_current will not be changed
	DLOG2 << "ReInitSimulatorByName()\n";
	if (fmuSim_length==1) {
		DLOG2 << "ReinitSimulatorbyname " << FMUPath << " not neede. only 1 simulation initialized.\n";
		return;
	}
	//DLOG1 << "reinitsimulatorbyname comparing " << FMUPath << " ?==?" <<
	 for (int i=0;i<fmuSim_length;i++){
		 if (strcmp(FMUPath,fmuSim_array[i]->fmufilename)==0) { 
			 fmuSim_current=fmuSim_array[i];
			 fmuSim_currentindex=i;
			 DLOG2 << "ReinitSimulatorbyname " << FMUPath << fmuSim_currentindex <<"\n";
			 return;
		 }

	 }
}

DLLEXPORTVOID ReInitSimulator(int index) {
	if (index<fmuSim_length)
      fmuSim_current=fmuSim_array[index];
	
}

/* disposes simulator, 
 closes logfile
 call once at the end */
DLLEXPORTVOID DisposeSimulator() {
	if (fmuSim_current->fmu!=NULL) {
	fmi1_import_destroy_dllfmu(fmuSim_current->fmu);

	fmi1_import_free(fmuSim_current->fmu);
	fmi_import_free_context(fmuSim_current->context);
	}
	fmuSim_current->fmu=NULL;
	_rmdir(fmuSim_current->fmutemppath);

	free (fmuSim_current);
	fmuSim_length--;
	//if the disposed simulator was last, then set current to the previous one
	if (fmuSim_length>0){
	if (fmuSim_currentindex == fmuSim_length) { 
		fmuSim_currentindex--;
		if (fmuSim_currentindex>-1) fmuSim_current=fmuSim_array[fmuSim_currentindex];
	} else {
		fmuSim_array[fmuSim_currentindex] = fmuSim_array[fmuSim_length]; //hold the last one to the current position
		fmuSim_current=fmuSim_array[fmuSim_currentindex];
	}
	}
	DLOG << "FMI disposed.\n";

	//DLOG.close();
}

DLLEXPORTVOID DisposeAllSimulator(){
	for (int i=0; i< fmuSim_length;i++){
		if (fmuSim_array[i]->fmu!=NULL) {
			fmi1_import_destroy_dllfmu(fmuSim_array[i]->fmu);

			fmi1_import_free(fmuSim_array[i]->fmu);
			fmi_import_free_context(fmuSim_array[i]->context);
		}
		fmuSim_array[i]->fmu=NULL;
		_rmdir(fmuSim_array[i]->fmutemppath);

		free (fmuSim_array[i]);
		//free (fmuSim_current);

	}
	fmuSim_length=0;
	fmuSim_currentindex=-1;

}


/* performs one step of simulation,
if the simulation exceeds the simulation endtime stops the running
*/
clock_t beginstep;
clock_t endstep;
double elapsed_secs;

DLLEXPORTVOID SetStepTime(double value) {
	fmuSim_current->hstep = value;
}

DLLEXPORTVOID InitializeSlave() 
{
	fmuSim_current->fmistatus = fmi1_import_initialize_slave(fmuSim_current->fmu, fmuSim_current->tstart, fmuSim_current->StopTimeDefined, fmuSim_current->tend);
	if(fmuSim_current->fmistatus != fmi1_status_ok) {
		DLOG <<"some ERRORs during fmi1_import_initialize_slave failed. Ignoring ...\n"; //ignore it for Hummod.
	}
}

//fixed bug - reset slave didn't work and cause parameters not set after initialization
void reset_slave() {
	/*DLOG1 << "terminate_slave() ";
	DLOG1.flush();
	fmi1_import_terminate_slave(fmu);
	*/
	DLOG1 << "reset_slave() \n";
	DLOG1.flush();
	fmuSim_current->fmistatus = fmi1_import_reset_slave(fmuSim_current->fmu);
	if(fmuSim_current->fmistatus != fmi1_status_ok) {
		DLOG <<"some ERRORs during fmi1_import_reset_slave failed. Ignoring ...\n"; //ignore it for Hummod.
	}

	//DLOG << "initialize_slave() \n";
	//DLOG.flush();
	//TODO remove the temp dir????
	//_mkdir(tmpPath);
	//global_tmp_path=tmpPath;
	InitializeSlave(); //moved from driver's client - TODO test bug parameter settings
	//DLOG << "Done - slave reset.\n";
	//DLOG.flush();
	/*jmstatus = fmi1_import_instantiate_slave(fmu, instanceName, fmuLocation, mimeType, timeout, visible, interactive);
	if (jmstatus == jm_status_error) {
		DLOG << "ERROR: fmi1_import_instantiate_model failed\n";
		sprintf(lasterrormessage,"ERROR: fmi1_import_instantiate_model failed");
		return;
	}*/

	//fmi1_import_instantiate_slave(fmu);
}
double epsilon = 1; // second tolerance in simulation reset times
double epsilon2 = 0.0001; // second tolerance in simulation reset times

DLLEXPORTVOID ResetSimulationTimes(double start, double step, double end) {
DLOG1 << "ResetSimulationTimes() start:"<< start <<" step:"<< step<<" end:"<<end <<" tcur:"<<fmuSim_current->tcur << " branch: ";
DLOG1.flush();
//if the simulator current time is start time - no reset is needed.
if (abs(fmuSim_current->tcur-start)<epsilon) {//if the simulator current time is start time - no reset is needed.
	DLOG1 <<"1 \n";
	DLOG1.flush();
	fmuSim_current->hstep = step;
	fmuSim_current->tend =end;
} else if (start>fmuSim_current->tcur) {//if the simulator current time is before requested start time - perform one step to be at requested start time
	DLOG1 <<"2 \n";
	DLOG1.flush();
	fmuSim_current->hstep = start-fmuSim_current->tcur;
	StepSimulation();
	fmuSim_current->hstep = step;
	fmuSim_current->tend = end;
} else if (start<epsilon) { //tcur is different and start is in the beginning - reset simulation
	DLOG1 <<"3 \n";
	DLOG1.flush();
	reset_slave(); 
	fmuSim_current->tstart = start;
	fmuSim_current->tcur = start;
	fmuSim_current->hstep = step;
	fmuSim_current->tend = end;
} else if ((fmuSim_current->tcur>0) && (start<fmuSim_current->tcur)) //go back with simulation - start is before current simulation time
{ 
	DLOG1 <<"4 \n";
	DLOG1.flush();
	reset_slave(); 
	fmuSim_current->tstart = start;
	fmuSim_current->tcur = 0;
	fmuSim_current->hstep = start-fmuSim_current->tcur;
	StepSimulation(); //TODO initialize is needed, yes
	fmuSim_current->hstep = step;
	fmuSim_current->tend = end;
} else {
	DLOG1 << "5 warning it should not be here \n";
}
 //not supported need to terminate and instantiate
 //InitializeSlave(); fix bug after initialization it is not possible to set some parameters. Must be initialized outside later
}

DLLEXPORTVOID StepSimulation() {
//	int dr;
//	double steptime;
	DLOG2 << "StepSimulation()";DLOG2.flush();
	//static fmi1_status_t fmistatus;	
	//beginstep = clock();
	DLOG2 << "step cur" << fmuSim_current->tcur << " hstep " << fmuSim_current->hstep;
	if (fmuSim_current->newStep) DLOG2 << " newstep true\n";
	else DLOG2 << " newstep false\n";
	DLOG2.flush();
	fmuSim_current->fmistatus = fmi1_import_do_step(fmuSim_current->fmu, fmuSim_current->tcur, fmuSim_current->hstep, fmuSim_current->newStep);
	//endstep = clock();
	//elapsed_secs = double(endstep - beginstep) / CLOCKS_PER_SEC;
	//DLOG2 << "took " << elapsed_secs << "seconds.\n";
	DLOG2.flush();
	fmuSim_current->tcur += fmuSim_current->hstep;
	if (fmuSim_current->tcur>fmuSim_current->tend) { 
		fmuSim_current->simulationstarted=false;
		//DLOG << "Simulation reached end of the time. Stopped.\n";
	}
}

DLLEXPORTVOID StepZeroSimulation() {
	//	int dr;
	//	double steptime;
	DLOG2 << "StepZeroSimulation()";DLOG2.flush();
	static fmi1_status_t fmistatus;	
	//beginstep = clock();
	DLOG2 << "step cur" << fmuSim_current->tcur << " step=0 (hstep " << fmuSim_current->hstep;
	if (fmuSim_current->newStep) DLOG2 << ") newstep true\n";
	else DLOG2 << ") newstep false\n";
	DLOG2.flush();
	fmistatus = fmi1_import_do_step(fmuSim_current->fmu, fmuSim_current->tcur, 0, fmuSim_current->newStep);
	//endstep = clock();
	//elapsed_secs = double(endstep - beginstep) / CLOCKS_PER_SEC;
	//DLOG2 << "took " << elapsed_secs << "seconds.\n";
	DLOG2.flush();
	//tcur += hstep; bug no step performed but tcur is increased
	if (fmuSim_current->tcur>fmuSim_current->tend) { 
		fmuSim_current->simulationstarted=false;
		//DLOG2 << "Simulation reached end of the time. Stopped.\n";
	}
}



/* Initialize simulation, 
loads the initial condition, sets simulation time to beginning
will overwrite all variable changes done before
call for simulating the model from initial condition

void InitSimulation(char * inputfilename) {

		DLOG << "InitSimulation() loading snapshot " << inputfilename << "\n";
		DymosimLoadSnapshot(simulator,inputfilename); //load snapshot from the beginning- then it is possible to change parameters before running simulation again 
		simulationstarted = false;
}
*/
DLLEXPORTVOID InitSimulation() {
	fmuSim_current->newStep = fmi1_true;
	
	//simulation parameters	
	
	DLOG1 << "InitSimulation() \n";
	char* buffer;

	// Get the current working directory: 
	if( (buffer = _getcwd( NULL, 0 )) == NULL )
		perror( "_getcwd error" );
	else
	{
		DLOG1 << "current directory:" << buffer << "\n";
		free(buffer);
	}

	DLOG1.flush();
	DLOG1 << "Version returned from FMU:   "<< fmi1_import_get_version(fmuSim_current->fmu) << "\n";
	DLOG1 << "Platform type returned:      "<< fmi1_import_get_types_platform(fmuSim_current->fmu) << "\n";

	//measuring time of simulation for debug purposes
	//clock_t begin = clock();
	fmuSim_current->fmuGUID = fmi1_import_get_GUID(fmuSim_current->fmu);
	DLOG1 << "GUID: " << fmuSim_current->fmuGUID <<"\n";

	
	fmuSim_current->jmstatus = fmi1_import_instantiate_slave(fmuSim_current->fmu, fmuSim_current->instanceName, fmuSim_current->fmuLocation, fmuSim_current->mimeType, fmuSim_current->timeout, fmuSim_current->visible, fmuSim_current->interactive);
	if (fmuSim_current->jmstatus == jm_status_error) {
		DLOG << "ERROR: fmi1_import_instantiate_model failed\n";
		DLOG.flush();
		sprintf_s(fmuSim_current->lasterrormessage,1024,"ERROR: fmi1_import_instantiate_model failed");
		return;
	}

	//change directory to the resources subdir of FMU - if it contains initialization files
	// fix of issue with Hummod. Hummod reads during initialization setup files - it must within FMU in resources subdir and it is extracted into tmp/resources subdir
	char resourcesPath[1024];
	strcpy_s(resourcesPath,1024,fmuSim_current->fmutemppath); //TODO fmu temp path global variable
	strcat_s(resourcesPath,1024,"/resources");
	_chdir(resourcesPath);

	InitializeSlave();
}

DLLEXPORTVOID StartSimulation() {
	//InitSimulation();
    fmuSim_current->simulationstarted=true; //after each InputRequestCompleted - StepSimulation() is automatically executed		
}

//resumes simulation running after previous stop
DLLEXPORTVOID ResumeSimulation() {
    fmuSim_current->simulationstarted=true; //after each InputRequestCompleted - StepSimulation() is automatically executed		
}

/* Disables automatic simulation step during reading variables - InputRequestCompleted */
DLLEXPORTVOID StopSimulation() {
    fmuSim_current->simulationstarted=false;		
	//? SaveSimulation("dsintermediate.txt");
	//simulationstarted = false;
}

DLLEXPORTVOID PauseSimulation() 
{
	fmuSim_current->simulationstarted = false;
}


DLLEXPORTVOID ContinueSimulation() {
		//InitSimulation("dsintermediate.txt");
		ResumeSimulation();
}

/* restarts simulation if some step was already executed,
it saves snapshot and reloads the saved snapshot as a new initial condition of started simulation */
DLLEXPORTVOID RestartSimulationIfNeeded() {
	/*if (simulationstarted) {
		DLOG << "RestartSimulation(); save snapshot and loads it to be able to change variables\n";
		SaveSimulation("dsintermediate.txt");
		InitSimulation("dsintermediate.txt");
	}*/
}
 /* writes value to the simulator */
DLLEXPORTVOID SetVariableValue(char * variableName, double value) {
		fmi1_status_t status;
		RestartSimulationIfNeeded();
		DLOG1 << "WriteRealToFMU " << variableName << " " << value << " ";
		fmi1_import_variable_t * variable = fmi1_import_get_variable_by_name(fmuSim_current->fmu,variableName);
		//TODO throw exception if variable is not found !!!
		fmi1_value_reference_t myvaluesRef = 0 ;
			if (variable != NULL) {
				myvaluesRef = fmi1_import_get_variable_vr(variable);
				fmi1_value_reference_t vr[] = {myvaluesRef};
		        fmi1_real_t value2[] = {value};
				status = fmi1_import_set_real(fmuSim_current->fmu,vr,1,value2);
		//bool ok = DymosimSetValue(simulator,handle[channelindex].category,handle[channelindex].index, value);
				DLOG2 << status << "\n";
			} else 
				DLOG1 << "variable not found in model FMU:" << variableName << "\n";

			
}
//TODO bug - returns 0 after simulation continues
DLLEXPORTVOID GetVariableValue(char * variableName, double * value) {
	fmi1_status_t status;
	fmi1_import_variable_t * variable = fmi1_import_get_variable_by_name(fmuSim_current->fmu,variableName);
	//TODO throw exception if variable is not found !!!
	fmi1_value_reference_t myvaluesRef = 0 ;
	if (variable != NULL) {
		myvaluesRef = fmi1_import_get_variable_vr(variable);
		fmi1_value_reference_t vr[] = {myvaluesRef};	
	    fmi1_import_get_real(fmuSim_current->fmu,vr,1,value);
		DLOG2 << "GetVariableValue " << variableName << " " << * value  << " ";
	} else if (_stricmp(variableName,"time")==0) {  
		memcpy(value,&fmuSim_current->tcur,sizeof(double));
		DLOG2 << "Time "<<fmuSim_current->tcur <<"\n";
	} //compare "case" not sensitive with time
	else 
	DLOG1 << "variable not found in model FMU:" << variableName << "\n" ;
}

//expect double null terminated array of strings
DLLEXPORTVOID GetVariableValues(const char ** variableNames,long variableNamesLength, double * values){//double * values[]) {
	DLOG2 << "GetVariableValues()\n";
	fmi1_status_t status;
	int i=0;
	int j=0;
	static fmi1_import_variable_t * variable[4096]; //TODO changed fixed size of array of pointers, probably
	static fmi1_value_reference_t myvaluesRef[4096];
	static fmi1_real_t myvalues[4096];
	//if (values==NULL) values = new double [variableNamesLength];
	//static const char * variableName[4096];
	//loop until the pszz is null - null terminated array of string - collect value references
	//j holds number of p
	//for (char * pszz = variableNames; *pszz; pszz += strlen(pszz) + 1) { 
	if (variableNamesLength>4096) {DLOG << "requested more variables than 4096\n"; return;}
	for (i=0;i<variableNamesLength;i++) { 
		//variableName[i]=variableNames[i]; 
		//DLOG2 << "getvariablevalues debug 1 " << i << "\n";
		//DLOG2.flush();
		variable[i]= fmi1_import_get_variable_by_name(fmuSim_current->fmu,variableNames[i]);
		if (variable[i]!=NULL) {
			//DLOG2 << "getvariablevalues debug 2 " << i <<"\n";
			//DLOG2.flush();
			myvaluesRef[j++] = fmi1_import_get_variable_vr(variable[i]);
		}
	//	i++; 
	}
	//const int variableNamesLength = i;
	const int valuesLength = j;
	//gets the values from model
	//DLOG2 << "get "<< j << " values from model ";
	//DLOG2.flush();
	fmi1_import_get_real(fmuSim_current->fmu,myvaluesRef,valuesLength,myvalues);

	//now translate to output - not found variables will be ignored, time will be set from simulation time
	// j holds index of known model variables, i holds index of all requested variables
	j=0;
	for (i=0;i<variableNamesLength;i++){
		if (variable[i] != NULL) {

			//fmi1_value_reference_t vr[] = {myvaluesRef};	
			values[i]=myvalues[j];
			j++;
			DLOG2 << "GetVariableValue " << variableNames[i] << " " << values[i]  << " ";
			DLOG2.flush();
		} else if (_stricmp(variableNames[i],"time")==0) {  
			//memcpy(value,&tcur,sizeof(double));
			values[i]= fmuSim_current->tcur;			
			DLOG2 << "Time "<<fmuSim_current->tcur << " "; //<<"\n";
			DLOG2.flush();
		} //compare "case" not sensitive with time
		else {
			DLOG1 << "variable not found in model FMU:" << variableNames[i] << "\n" ;
			DLOG2.flush();
		    values[i]=0; // requested variable value will be null;
		}
	}
	//for (i=0;i<variableNamesLength;i++) DLOG2 << "\n values[" << i <<"] " << values[i] << "\n";
	DLOG2 << "\n";
	DLOG2.flush();
	//return values;
}
DLLEXPORTVOID GetVariableValuesTest(double * value,long length){
	for (int i=0;i<length;i++){

    value[i]  = 3.1415926535+i;
	}

}
/*DLLEXPORTVOID GetVariableValues2(char * variableNames, double * values){//double * values[]) {
	fmi1_status_t status;
	int i=0;
	int j=0;
	fmi1_import_variable_t * variable[4096]; //TODO changed fixed size of array of pointers, probably
	fmi1_value_reference_t myvaluesRef[4096];
	fmi1_real_t myvalues[4096];
	//static double values[4096];
	char * variableName[4096];
	//loop until the pszz is null - null terminated array of string - collect value references
	//j holds number of p
	for (char * pszz = variableNames; *pszz; pszz += strlen(pszz) + 1) { 
		//for (i=0;i<variableNamesLength;i++) { 
		variableName[i]=pszz;//variableNames[i]; 
		variable[i]= fmi1_import_get_variable_by_name(fmu,variableName[i]);
		if (variable[i]!=NULL) {
			myvaluesRef[j++] = fmi1_import_get_variable_vr(variable[i]);
		}
		i++; 
	}
	const int variableNamesLength = i;
	const int valuesLength = j;
	//gets the values from model
	fmi1_import_get_real(fmu,myvaluesRef,valuesLength,myvalues);

	//now translate to output - not found variables will be ignored, time will be set from simulation time
	// j holds index of known model variables, i holds index of all requested variables
	j=0;
	for (i=0;i<variableNamesLength;i++){
		if (variable[i] != NULL) {

			//fmi1_value_reference_t vr[] = {myvaluesRef};	
			values[i]=myvalues[j];
			j++;
			DLOG2 << "GetVariableValue " << variableName << " " << values[j]  << " ";
		} else if (_stricmp(variableName[i],"time")==0) {  
			//memcpy(value,&tcur,sizeof(double));
			values[i]= tcur;
			j++;
			DLOG2 << "Time "<<tcur <<"\n";
		} //compare "case" not sensitive with time
		else 
			DLOG1 << "variable not found in model FMU:" << variableName << "\n" ;
		values[i]=0; // requested variable value will be null;
	}		
	//return values;
}*/