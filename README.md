Physio.FMUDriver4CW
===================

FMU Driver for Control Web and .NET


Project structure
=================
``` doc ``` Documentation for ControlWeb integration
``` include ``` Header files
``` lib ``` Third party libraries - FMI library from Modelon
``` project ``` project files to compile under MS VS2010
``` src ``` C source files
``` test ``` Test project of ControlWeb, Modelica model and FMI driver
``` thirdparty ``` Sources of thirdparty library

CW2FMIDriver
============

  * CW2FMIGenerator.exe - generator of PAR file from FMU package
  * CW2FMIDriver.dll - driver for ControlWeb 3+, .NET
  * fmilib_shared.dll - fmilibrary redistribution package from fmi-library.org

Installation instruction
========================

* unzip the zip package into your folder e.g. C:\CW2FMIHummod
* export FMO from model
  * for Dymola version 2012 needs to be for co-simulation and ImplementationTag=true.
  * Advanced.FMI.IncludeImplementationTag = true; 
  * menu -> Simulation -> Translate -> FMU
    translateModelFMU("HumMod.HumMod_GolemEdition", false);
* copy FMU into the installation folder of CW2FMIDriver
* generate PAR file for FMU
  * in commandline (cmd.exe)
    cd c:\CW2FMIHummod
  * CW2FMIGenerator.exe [mymodel.fmu] [mymodel.par]
    * where mymodel.fmu is name of the FMU package file, mymodel.par is name of newly created PAR file
    * e.g.:
    CW2FMIGenerator.exe HumMod_Hummod_GolemEdition.fmu hummod.par
 
* workaround for the is issue #107, copy DLL from tempfmu/binaries/win32/ to c:\windows\system32
* launch ControlWeb, install driver CW2FMIDriver.DLL,  load PAR file generated previously and DMF file (manually edited before)
