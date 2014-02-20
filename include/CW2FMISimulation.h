
//specific functions of a driver
#define DLLEXPORT __declspec( dllexport )
#define DllIMPORT  __declspec(dllimport)
#define DLLEEXPORTVOID DLLEXPORT void __cdecl
extern "C" {
__declspec( dllexport ) void __cdecl DisposeSimulator();
__declspec( dllexport ) void __cdecl StepSimulation();
__declspec( dllexport ) void __cdecl StartSimulation();
__declspec( dllexport ) void __cdecl StopSimulation();
__declspec( dllexport ) void __cdecl InitSimulation(); 
__declspec( dllexport ) void __cdecl InitializeSlave(); 
__declspec( dllexport ) void __cdecl RestartSimulationIfNeeded();
__declspec( dllexport ) void __cdecl PauseSimulation();
__declspec( dllexport ) void __cdecl ResumeSimulation();
__declspec( dllexport ) void __cdecl ContinueSimulation();
__declspec( dllexport ) void __cdecl InitSimulator(const char * FMUPath, const char * tmpPath);
__declspec( dllexport ) void __cdecl GetVariableValue(char * variableName, double * value);
__declspec( dllexport ) void __cdecl GetVariableValues(char ** variableNames, int variableNamesLength, double * values);
__declspec( dllexport ) void __cdecl SetVariableValue(char * variableName, double value);
__declspec( dllexport ) void __cdecl SetStepTime(double value);
__declspec( dllexport ) void __cdecl ResetSimulationTimes(double start, double step, double end);
}