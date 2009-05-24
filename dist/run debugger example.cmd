@echo off
set PATH=%PATH%;%CD%\bin
set firefox="%ProgramFiles%\Mozilla Firefox\firefox.exe"
if not exist %firefox% (
 echo Unable to run firefox at %firefox%.
 pause
 exit
)
start %firefox% bin\debugger.xul
jshost bin/debugger.js examples/helloworld.js
