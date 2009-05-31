@echo off
set PATH=%PATH%;%CD%\bin
set firefox=%ProgramFiles%\Mozilla Firefox\firefox.exe
if not exist "%firefox%" (
 echo WARNING: Unable to run firefox at %firefox%.
 echo Try to load by hand the file .\bin\debugger.xul in your Firefox ^(using drag'n'drop^).
) else (
 echo Starting debugger user interface...
 start "" "%firefox%" ".\bin\debugger.xul"
)
echo Starting JavaScript program...
jshost -u bin/debugger.js examples/testForDebugger.js