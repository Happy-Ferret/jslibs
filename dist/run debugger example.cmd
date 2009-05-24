@echo off
set PATH=%PATH%;%CD%\bin
set firefox="%ProgramFiles%\Mozilla Firefox\firefox.exe"
if not exist %firefox% (
 echo Unable to run firefox at %firefox%.
 pause
 exit
)

echo Starting debugger user interface...
start %firefox% bin\debugger.xul

echo Starting JavaScript program...
jshost bin/debugger.js examples/testForDebugger.js
