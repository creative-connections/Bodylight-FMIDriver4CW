cd /d "D:\patfyz\CWFmiDriver\CWFmiDriver" &msbuild "CWFmiDriver.vcxproj" /t:sdvViewer /p:configuration="Debug" /p:platform=Win32
exit %errorlevel% 