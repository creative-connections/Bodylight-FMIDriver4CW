// CW2FMIGenerator2.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
//standard libraries
#include <Windows.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <atlstr.h>
#include <direct.h>

#include "Cw2FmiDriver.h"


using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	cout << "CW2FMIGenerator generates INI files for Control Web from existing FMU of Modelica model";
	if (argc < 3) {
		cout << "usage:\nCW2FMIGenerator [file1] [file2]\n        [file1] filename of existing FMU file\n       [file2] filename of non existing PAR file which will be generated";
		return 0;
	}
	LPTSTR filename=argv[2];
	LPTSTR myfmufilename=argv[1];
	USES_CONVERSION;
	LPTSTR myfmutemppath=A2T("tempfmu");
	CreateParameterFile(filename,myfmufilename,myfmutemppath);
	return 0;
}

