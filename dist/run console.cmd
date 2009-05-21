@echo off
set PATH=%PATH%;%CD%\bin
cd examples
echo available tests:
echo ---------------
dir /B *.js
echo.
echo Try: jshost ^<test name^>.js
echo.
cmd /k
