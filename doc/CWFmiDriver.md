# Control Web FMI 2.0 Driver

This driver functions as an interface between Control Web and a compiled binary FMU version 2.0, more information about the Functional Mock-up Interface (FMI) standards can be found at https://fmi-standard.org.

This documentation will be refering to the FMI 2.0 specification chapters for example (FMI 2.2.4). The FMI 2.0 Specification can be obtained at https://fmi-standard.org/downloads/ and a copy of the .pdf file is supplied with the driver .dll.

## Usage
You will need a FMU (.fmu) file containing executable binaries and modelDescription.xml.  Load the driver with a correct configuration in parameterFile.par and channel configuration in the .dmf file. 

This is an example of a  minimal viable parameterFile.par.
```json
{
  "fmu": "myFirstFMU.fmu",
  "channels": {
    "1": "comp1.subcomponent.param",
    "2": "comp2.param1",
  },
}
```
If you have specified all of the required parameters in the parameterFile.par you can invoke the initialize function.

> `core.DriverQueryProc('FMU', 'initialize', '');`

After which channels will become readable and writeable. You can advance the simulation by 
      
> `core.DriverQueryProc('FMU', 'doStep', 0.001);`

In this case it will advance by 0.001 time units, whichever they might be.

## Configuration
Parameter file referred to as `parameterFile.par` is JSON formatted. This is an example of an complete parameter file:

```json
{
  "workingDirectory": "",
  "fmu": "myFirstFMU.fmu",
  "fmuDestinationDirectory": "",
  "channels": {
    "1": "comp1.subcomponent.param",
    "2": "comp2.param1",
    "3": "comp2.param2",
    "4": "comp2.isRunning",
  },
  "initialValues": {
    "1": 1.5,
    "4": true,
    "2": 1
  },
  "instanceName": "FMUInstanceName",
  "tolerance": 0.0001,
  "startTime": 0.0,
  "visible": true,
  "resourceLocation": "File:\\",
  "driverLoggingOn": true,
  "driverLogDirectory": "logs",
  "driverLogFileName": "driver",
  "driverLogVerbosity": "warning",
  "fmuLogDirectory": "logs",
  "fmuLogFileName":  "fmu",
  "fmuLoggingOn": true,
  "fmuLogCategories": [
    "logEvents", 
    "logStatusWarning",
    "logStatusError"
  ]
}
```

* __workingDirectory__ `:string` (optional) defines path to working directory.  All subsequent relative file and directory definitions will be relative to this directory.
  * Omitted or empty parameter `workingDirectory` will default to the directory of `parameterFile.par`. 
  * Relative path for this parameter will be relative to the directory of `parameterFile.par`.
  * Directory SHOULD be writeable otherwise you have to specify different writeable directories for parameters below.

* __fmu__ `:string` absolute or relative path to the `.fmu` file.
  * Invalid value for this parameter results in failure to initalize the driver.

* __fmuDestinationDirectory__ `:string` (optional, default: "fmu\") absolute or relative path to a directory where should the `.fmu` file be extracted.

  * Different instances of the driver MUST have different `fmuDestinationDirectory` otherwise undefined behaviour might occur.
  * (TODO) `fmuDestinationDirectory` will be removed after driver exits.

* __channels__ `{"channel": "ScalarVariable.name"}` defines chanel mapping, 
  * (TODO) driver fails to initialize if no such _ScalarVariable_ (FMI 2.2.7) exists in the FMU.

* __initialValues__ `{"channel": real|int|bool}` (optional)
   Specifies values for channels which will be set after `fmi2EnterInitializationMode` (FMI 2.1.6)  and before `fmi2ExitInitializationMode` (FMI 2.1.6)
  * (TODO) driver fails to initialize if channel mapping is wrong, or the ScalarVariable is not modifiable

### FMU Initialization

All of these variables can be changed in Control Web, see section on driver control functions 

* Parameters of __`fmi2Instantiate`__ (FMI 2.1.5)
  * __instanceName__ `:string` (optional, default: name extracted from modelDescription.xml) Specifies the instance name, currently it is mainly useful as an identifier in log files.
  * __visible__ `:boolean` (optional, default: `true`)
    * Argument `visible = fmi2False` defines that the interaction with the user should be reduced to a minimum (no application window, no plotting, no animation, etc.), in other words the FMU is executed in batch mode. If `visible = fmi2True`, the FMU is executed in interactive mode and the FMU might require to explicitly acknowledge start of simulation / instantiation / initialization (acknowledgment is non-blocking). (FMI 2.1.5) 
	* This functionality is not controlled by driver.
  * __resourceLocation__ `:string` (optional, default: `"file:\\"`)
    * Argument fmuResourceLocation is an URI according to the IETF RFC3986 syntax to indicate the location to the “resources” directory of the unzipped FMU archive. The following schemes MUST be understood by the FMU: 
      * Mandatory: “file” with absolute path (either including or omitting the authority component)
      *	Optional: “http”, “https”, “ftp”
      * Reserved: “fmi2” for FMI for PLM.
    * _[Example: An FMU is unzipped in directory “C:\temp\MyFMU”, then fmuResourceLocation = "file:///C:/temp/MyFMU/resources" or "file:/C:/temp/MyFMU/resources". Function fmi2Instantiate is then able to read all needed resources from this directory, for example maps or tables used by the FMU.]_ (FMI 2.1.5)

* Parameters of __`fmi2SetupExperiment`__ (FMI 2.1.6)
  * __startTime__ `:real` 
    * Argument startTime is the fixed initial value of the independent variable5 [if the independent variable is “time”, startTime is the starting time of initializaton]. 
  * __tolerance__ `:real` (optional, default: `toleranceDefined = false`)
    * If tolerance is defined then the communication interval of the slave is controlled by error estimation. In case the slave utilizes a numerical integrator with variable step size and error estimation, it is suggested to use “tolerance” for the error estimation of the internal integrator (usually as relative tolerance).  An FMU for Co-Simulation might ignore this argument. (FMI 2.1.6)
	* Note: previous experimentation with FMUs showed that the speed of `fmi2DoStep` varies greately dependent on the tolerance value, it is highly recommended to check how tolerance affects the performance of your particular FMU solver.

### Logging
There are two potential sources of logs in the application, the FMU internal logging output and the driver logging output. 

Output from the FMU might contain information about runtime occurences, such as division by zero and other integration failures. The driver log provides information about Control Web integration issues, such as improperly configured channels, setting read only variables and other.

* __fmuLoggingOn__ `:bool` (optional, default: false)

* __fmuLogDirectory__ `:string` (optional, default: "logs\fmu\") specifies folder for FMU logging

* __fmuLogFileName__ `:string` (optional, default: "fmu") specifies log file name. 
  * Name is appended with timestamp (`name_yyyymmdd_hhmmss`) 

* __driverLoggingOn__  `:bool` (optional, default: false)

* __driverLogDirectory__ `:string` (optional, default: "logs\fmu\") specifies folder for driver logging

* __driverLogFileName__ (optional, default: "CWFmiDriver") `:string` specifies log file name. 
  * Name is appended with timestamp (`name_yyyymmdd_hhmmss`) 
  * (TODO/CHECK) Log file will gets flushed to disk every 10MiB, or event with higher severity than info.

* __driverLogVerbosity__ `:string` (optional, default: "warning") specifies logging verbosity
  * `"error"` only errors will be logged, errors are not recoverable
  * `"warning"` warnings and errors will be logged
  * `"debug"` some function calls will be logged, interaction with FMU will be logged. 
  * `"trace"` hihest level of verbosity. This setting can generate logs at MiB/s speeds, production use not recommended.

* Parameters of __`fmi2SetDebugLogging`__ (FMI 2.1.5)
  * (TODO) __fmuLogCategories__ `["category1", "category2"]` (optional)
    * If set then only debug messages according to this argument shall be printed via the logger function. The allowed values of `fmuLogCategories` are defined by the modeling environment that generated the FMU. Depending on the generating modeling environment, none, some or all allowed values for `fmuLogCategories` for the FMU are defined in the modelDescription.xml file via element `fmiModelDescription.LogCategories`, see FMI 2.2.4;
	* If this parameter is left out or is empty, then all of the logging levels for the FMU will be logged. Log categories no in the modelDescription.xml file will be discarded, consequently if this parameter is set, but none of the categories are present in the modelDesceription.xml file then all of the FMU logging categories will be logged.

## Driver control functions

### initialize () 
Initializes the FMU with loaded configuration values (either from parameterFile.par or overriden by control functions).

On the first call it will `fmi2Instantiate` the FMU and then it will `fmi2SetupExperiment`. After that it `fmi2EnterInitializationMode`, loads the initial channel values specified by `initialValues` and then exits the initialization mode. After this, it is possible to read and write to channels and call `doStep()`

* Must be called before any attempt to read or write to channels.
* Calling `initialize()` after `reset()` will not reinstantiate the FMU (paramaters instanceName, resourceLocation and visible will not change).

### doStep (stepSize : Real)

Alias for `fmi2DoStep` (FMI 4.2.2). Function advances the simulation by time specified by stepSize. 

Channel writes are buffered and only applied just before `fmi2DoStep` is invoked on the FMU. This is because some tested FMU implementations do not handle data read correctly after data write within a single time step. (Possible TODO: make this behaviour configurable??)

Currently `noSetFMUStatePriorToCurrentPoint` is set to `fmi2True`. So it is not possible to go back in simulation time. 

### getLastStepStatus () : integer
Returns fmi2Status (FMI 2.1.3) for the last doStep operation.
* 0: fmi2OK
* 1: fmi2Warning
* 2: fmi2Discard
* 3: fmi2Error
* 4: fmi2Fatal
* 5: fmi2Pending

### reset ()
Alias for `fmi2Reset` (FMI 2.1.6). It is called by the environment to reset the FMU after a simulation run. The FMU goes into the same state as if `initialize()` would not have been called. All variables have their default values. Before starting a new run `initialize()` MUST be called.

### terminate ()
Alias for `fmi2Terminate` (FMI 2.1.6). Informs the FMU that the simulation run is terminated. After calling this function, the final values of all variables can be inquired with the fmi2Get*() functions. It is not allowed to call this function after one of the functions returned with a status flag of fmi2Error or fmi2Fatal. It is not allowed to call `doStep()` after calling this function.

### getTime () : Real
  Returns simulation time.

### setInstanceName (name : String) 
  * Overrides parameter file value `instanceName`
  * Must be called before `initialize()`, can not be called after `reset()`.

### setResourceLocation (resourceLocation : String) 
  * Overrides parameter file value `resourceLocation`
  * Must be called before `initialize()`, can not be called after `reset()`.

### setVisible (visible : Boolean) 
  * Overrides parameter file value `visible`
  * Must be called before `initialize()`, can not be called after `reset()`.

### setTolerance (tolerance : Real) 
  * Overrides parameter file value `tolerance`
  * Must be called before `initialize()`, or after `reset()`.

### setStartTime (startTime : Real) 
  * Overrides parameter file value `startTime`
  * Must be called before `initialize()`, or after `reset()`.

### setFmuLoggingOn (value : Boolean)
  * Overrides parameter file value `fmuLoggingOn`
  * Can be called at any time.

### (TODO) setDriverLogVerbosity (verbosity : String)
  * Overrides parameter file value `driverLogVerbosity`
  * Can be called at any time.

## Requirements
Minimum supported client: Windows 8

Minimum supported server: Windows Server 2012

