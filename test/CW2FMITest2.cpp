// CW2FMITest2.cpp : main project file.

//#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

//#include "config_test.h"

#include <fmilib.h>
#include <ctime>
#include <direct.h>

#include "StdAfx.h"

#define BUFFER 1000
#define CTEST_RETURN_FAIL 1
#define CTEST_RETURN_SUCCESS 0

void importlogger(jm_callbacks* c, jm_string module, jm_log_level_enu_t log_level, jm_string message)
{
	printf("module = %s, log level = %s: %s\n", module, jm_log_level_to_string(log_level), message);

}

/* Logger function used by the FMU internally */

void fmilogger(fmi1_component_t c, fmi1_string_t instanceName, fmi1_status_t status, fmi1_string_t category, fmi1_string_t message, ...)
{
	char msg[BUFFER];
	va_list argp;	
	va_start(argp, message);
	vsprintf(msg, message, argp);
	fmi1_log_forwarding_v(c, instanceName, status, category, message, argp);
	va_end(argp);
}


void printVariableInfo(fmi1_import_t* fmu,
	fmi1_import_variable_t* v) {
		fmi1_base_type_enu_t bt;
		printf("Variable name: %s\n", fmi1_import_get_variable_name(v));
		printf("Description: %s\n", fmi1_import_get_variable_description(v));
		printf("VR: %d\n", fmi1_import_get_variable_vr(v));
		printf("Variability: %s\n", fmi1_variability_to_string(fmi1_import_get_variability(v)));
		printf("Causality: %s\n", fmi1_causality_to_string(fmi1_import_get_causality(v)));

		bt = fmi1_import_get_variable_base_type(v);
		printf("Base type: %s\n", fmi1_base_type_to_string(bt));

		//printTypeInfo(fmi1_import_get_variable_declared_type(v));
		/*if(bt == fmi1_base_type_real) {
			fmi1_import_real_variable_t *rv = fmi1_import_get_variable_as_real(v);
			fmi1_import_unit_t * u = fmi1_import_get_real_variable_unit(rv);
			fmi1_import_display_unit_t * du = fmi1_import_get_real_variable_display_unit(rv);
			printf("Unit: %s, display unit: %s\n", u ? fmi1_import_get_unit_name(u):0, du?fmi1_import_get_display_unit_name(du):0);
		}

		if(fmi1_import_get_variable_has_start(v)) {
			printf("There is a start value, fixed attribute is '%s'\n", (fmi1_import_get_variable_is_fixed(v))?"true":"false");

			switch(fmi1_import_get_variable_base_type(v)) {
			case fmi1_base_type_real: {
				fmi1_import_real_variable_t *rv = fmi1_import_get_variable_as_real(v);
				printf("start =%g\n", fmi1_import_get_real_variable_start(rv));
				break;
									  }
			case fmi1_base_type_int:{
				printf("start =%d\n", fmi1_import_get_integer_variable_start(fmi1_import_get_variable_as_integer(v)));
				break;
									}
			case fmi1_base_type_bool:{
				printf("start = %d\n", fmi1_import_get_boolean_variable_start(fmi1_import_get_variable_as_boolean(v)));
				break;
									 }
			case fmi1_base_type_str:{
				printf("start = '%s'\n", fmi1_import_get_string_variable_start(fmi1_import_get_variable_as_string(v)));
				break;
									}
			case fmi1_base_type_enum:{
				printf("start = %d\n", fmi1_import_get_enum_variable_start(fmi1_import_get_variable_as_enum(v)));
				break;
									 }
			default:
				printf("Error in fmiGetBaseType()\n");
			}
		}
		if(fmi1_import_get_variable_alias_kind(v) != fmi1_variable_is_not_alias) {
			printf("The variable is aliased to %s\n",
				fmi1_import_get_variable_name( fmi1_import_get_variable_alias_base(fmu, v)));
		}
		else {
			printf("The variable is not an alias\n");
		}
		{
			fmi1_import_variable_list_t* vl = fmi1_import_get_variable_aliases(fmu, v);
			size_t n = fmi1_import_get_variable_list_size(vl);
			unsigned i = (unsigned)n;
			//assert( n == i);
			if(n>1) {
				printf("Listing aliases: \n");
				for(i = 0;i<n;i++)
					printf("\t%s\n",fmi1_import_get_variable_name(fmi1_import_get_variable(vl, i)));
			}
			fmi1_import_free_variable_list(vl);
		}
		{
			fmi1_import_variable_list_t* vl = fmi1_import_get_direct_dependency( fmu, v);
			size_t n = 0;
			unsigned i;
			if(vl) 
				n = fmi1_import_get_variable_list_size(vl);
			i = (unsigned)n;
			//assert( n == i);		
			if(n>0) {
				printf("Listing direct dependencies: \n");
				for(i = 0;i<n;i++)
					printf("\t%s\n",fmi1_import_get_variable_name(fmi1_import_get_variable(vl, i)));
			}
			fmi1_import_free_variable_list(vl);
		}*/
}


void do_exit(int code)
{
	printf("Press 'Enter' to exit\n");
	 getchar();
	exit(code);
}

int test_simulate_cs(fmi1_import_t* fmu, const char * ref1, const char * ref2)
{
	fmi1_status_t fmistatus;
	jm_status_enu_t jmstatus;

	fmi1_string_t instanceName = "CW2FMITest";
	fmi1_string_t fmuGUID;
	fmi1_string_t fmuLocation = "";
	fmi1_string_t mimeType = "";
	fmi1_real_t timeout = 0.0;
	fmi1_boolean_t visible = fmi1_false;
	fmi1_boolean_t interactive = fmi1_false;
	/*	fmi1_boolean_t loggingOn = fmi1_true; */

	/* fmi1_real_t simulation_results[] = {-0.001878, -1.722275}; */
	//fmi1_real_t simulation_results[] = {0.0143633,   -1.62417};
	fmi1_value_reference_t compare_real_variables_vr[] = {atoi(ref1), atoi(ref2)}; //637572872 637573117, liverO2.O2Tissue.pO2 - 637571902
//	size_t k;

	fmi1_real_t tstart = 0.0;
	fmi1_real_t tcur = tstart;
	fmi1_real_t hstep = 10000;
	fmi1_real_t tend = 2419200.0; //28 days
	fmi1_boolean_t StopTimeDefined = fmi1_false;

/*	if (sizeof(compare_real_variables_vr)/sizeof(fmi1_value_reference_t) != sizeof(simulation_results)/sizeof(fmi1_real_t)) {
		printf("Number of simulation values and reference values are different\n");
		do_exit(CTEST_RETURN_FAIL);
	}
*/
	printf("Version returned from FMU:   %s\n", fmi1_import_get_version(fmu));
	printf("Platform type returned:      %s\n", fmi1_import_get_types_platform(fmu));

	clock_t begin = clock();
	fmuGUID = fmi1_import_get_GUID(fmu);
	printf("GUID:      %s\n", fmuGUID);


	jmstatus = fmi1_import_instantiate_slave(fmu, instanceName, fmuLocation, mimeType, timeout, visible, interactive);
	if (jmstatus == jm_status_error) {
		printf("fmi1_import_instantiate_model failed\n");
		do_exit(CTEST_RETURN_FAIL);
	}
	//change directory to the resources subdirectory of FMU - resources contains some initialization data
	//strcat(&fmulocation,"/resources");
	//chdir(fmuLocation);

	fmistatus = fmi1_import_initialize_slave(fmu, tstart, StopTimeDefined, tend);
	if(fmistatus != fmi1_status_ok) {
		printf("fmi1_import_initialize_slave failed\n");
		//do_exit(CTEST_RETURN_FAIL);
	}
	fmi1_import_variable_list_t* vl = fmi1_import_get_variable_list(fmu);
	size_t nv = fmi1_import_get_variable_list_size(vl);
	size_t i;
	//i = (unsigned)nv;
	//assert(i == nv);
	printf("There are %d variables in total. Soma variables listed: \n",nv);
	for(i = 0; i < nv; i+=1000) {
		fmi1_import_variable_t* var = fmi1_import_get_variable(vl, i);
		if(!var) {
			printf("Something wrong with variable %d\n",i);
			//do_exit(1);
		}
		else {
			//WriteVariableRow(&parameterFile,var);
				printVariableInfo(fmu, var);
				//testVariableSearch(fmu, var);
		}
	}
	fmi1_import_free_variable_list(vl);
	tcur = tstart;
	printf("%10s %10s\n", "ref1", "ref2");

	//code_to_time();

	clock_t end;// = clock();
	double elapsed_secs;// = double(end - begin) / CLOCKS_PER_SEC;
	int tick =24193;
	int percent = ((tend - tcur)/hstep) / 100;
	int prom=0;
	while (tcur < tend) {
		fmi1_boolean_t newStep = fmi1_true;
#if 0 /* Prints a real value.. */
		fmi1_real_t rvalue;
		fmi1_value_reference_t vr = 0;

		fmistatus = fmi1_import_get_real(fmu, &vr, 1, &rvalue);
		printf("rvalue = %f\n", rvalue);
#endif 
		fmistatus = fmi1_import_do_step(fmu, tcur, hstep, newStep);

	
		if (tick>percent) //1 promile of computation
		{
			tick=0;
			fmi1_real_t val[2];
			fmi1_import_get_real(fmu, compare_real_variables_vr, 2, val);
			end = clock();
			elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
			printf("%10g %10g  %d percent of computation takes %10g seconds.\n", val[0],val[1],prom, elapsed_secs);
			prom++;
		} else tick++;
		tcur += hstep;
	}

	end = clock();
	elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	printf("Simulation finished. Computation time %10g seconds.\n", elapsed_secs);

	//fmistatus = fmi1_import_terminate_slave(fmu);

	fmi1_import_free_slave_instance(fmu);

	return 0;
}

int main(int argc, char *argv[])
{
	fmi1_callback_functions_t callBackFunctions;
	const char* FMUPath;
	const char* tmpPath;
	const char* ref1;
	const char* ref2;
	jm_callbacks callbacks;
	fmi_import_context_t* context;
	fmi_version_enu_t version;
	jm_status_enu_t status;
	int k;

	fmi1_import_t* fmu;	

	if(argc < 5) {
		printf("Usage: %s <fmu_file> <temporary_dir> <ref1> <ref2>\n", argv[0]);
		do_exit(CTEST_RETURN_FAIL);
	} 
	for (k = 0; k < argc; k ++)
		printf("argv[%d] = %s\n", k, argv[k]);

	FMUPath = argv[1];
	tmpPath = argv[2];
	ref1 = argv[3];
	ref2 = argv[4];


	callbacks.malloc = malloc;
	callbacks.calloc = calloc;
	callbacks.realloc = realloc;
	callbacks.free = free;
	callbacks.logger = importlogger;
	callbacks.log_level = jm_log_level_debug;
	callbacks.context = 0;

	callBackFunctions.logger = fmi1_log_forwarding;
	callBackFunctions.allocateMemory = calloc;
	callBackFunctions.freeMemory = free;

#ifdef FMILIB_GENERATE_BUILD_STAMP
	printf("Library build stamp:\n%s\n", fmilib_get_build_stamp());
#endif

	context = fmi_import_allocate_context(&callbacks);

	version = fmi_import_get_fmi_version(context, FMUPath, tmpPath);

	if(version != fmi_version_1_enu) {
		printf("Only version 1.0 is supported so far\n");
		do_exit(CTEST_RETURN_FAIL);
	}

	fmu = fmi1_import_parse_xml(context, tmpPath);

	if(!fmu) {
		printf("Error parsing XML, exiting\n");
		do_exit(CTEST_RETURN_FAIL);
	}


	status = fmi1_import_create_dllfmu(fmu, callBackFunctions, 1);
	if (status == jm_status_error) {
		printf("Could not create the DLL loading mechanism(C-API) (error: %s).\n", fmi1_import_get_last_error(fmu));
		do_exit(CTEST_RETURN_FAIL);
	}
	//change dir to resources inside FMU - the initial values will be read from there
	char resourcesPath[256];
	strcpy(resourcesPath,tmpPath);
	strcat(resourcesPath,"/resources");
	_chdir(resourcesPath);

	test_simulate_cs(fmu,ref1,ref2);

	fmi1_import_destroy_dllfmu(fmu);

	fmi1_import_free(fmu);
	fmi_import_free_context(context);

	printf("Everything seems to be OK since you got this far=)!\n");

	do_exit(CTEST_RETURN_SUCCESS);

	return 0;
}
