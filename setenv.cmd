@echo off
echo Setting jslibs %1 environment.
cmd /k set PATH=%CD%\%1;%PATH%
