@echo off
echo Setting jslibs environment to %1
cd /D %~d0%~p0
set PATH=%~f1;%PATH%
cmd /k
