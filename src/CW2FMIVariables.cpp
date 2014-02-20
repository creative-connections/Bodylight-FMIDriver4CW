//standard libraries
#include <Windows.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <atlstr.h>
#include <direct.h>

#include "Cw2FmiDriver.h"
//#include "StdAfx.h"

fmi1_value_reference_t GetVariableReference(char * variableName) {
	fmi1_import_variable_t * variable = fmi1_import_get_variable_by_name(fmuSim_current->fmu,variableName);
	//TODO throw exception if variable is not found !!!
	return fmi1_import_get_variable_vr(variable);
}

/* reads value from simulator, stores it to the position determined for channel  */
fmi1_real_t GetValueFromFMU(int channelindex) {
	DLOG << "GetValueFromFMU("<< channelindex <<")\n";
	//fix bug #95 "time" variable doesn't return time 
	//ChannelValue[channelindex] = (handle[channelindex].index==-1)?DymosimGetTime(simulator):DymosimGetValue(simulator,handle[channelindex].category,handle[channelindex].index);
	fmi1_value_reference_t vr[] = {fmuSim_current->valuesRef[channelindex]};
	fmi1_real_t value[1];
	DLOG << "  valuesRef[channelindex]" << fmuSim_current->valuesRef[channelindex] <<"\n";
	fmi1_import_get_real(fmuSim_current->fmu,vr,1,value);

//	valuesArr[channelindex] = value[0];
	return value[0];
		//DymosimGetValue(handle[channelindex]);
}

void WriteDoubleToFMU( unsigned channelindex, double value) 
{
	fmi1_status_t status;
	RestartSimulationIfNeeded();
	DLOG << "WriteRealToFMU " << value << " ";
	fmi1_value_reference_t vr[] = {fmuSim_current->valuesRef[channelindex]};
	fmi1_real_t value2[] = {value};
	status = fmi1_import_set_real(fmuSim_current->fmu,vr,1,value2);
	//bool ok = DymosimSetValue(simulator,handle[channelindex].category,handle[channelindex].index, value);
	DLOG << status << "\n";
	//throw std::exception("The method or operation is not implemented.");
}

/* gets the value from the array, which was updated in the call GetValueFromDymola previous step */
/*double ReadDoubleFromCache(int channelindex) {
	//GetValueFromDymola(channelindex);
	return inputrvaluesArr[channelindex];
}*/

/* not implemented */
char * ReadStringFromCache(int channelindex) {
	DLOG << "not implemented read string from FMU\n";
	return "not implemented";
}


/* writes value to the simulator */
void WriteRealToFMU(int channelindex, float value) {
	fmi1_status_t status;
	RestartSimulationIfNeeded();
	DLOG << "WriteRealToFMU " << value << " ";
	fmi1_value_reference_t vr[] = {fmuSim_current->valuesRef[channelindex]};
	fmi1_real_t value2[] = {value};
	status = fmi1_import_set_real(fmuSim_current->fmu,vr,1,value2);
	//bool ok = DymosimSetValue(simulator,handle[channelindex].category,handle[channelindex].index, value);
	DLOG << status << "\n";
}

void WriteStringToFMU(int channelindex, void * value) {
	//not implemented
}
