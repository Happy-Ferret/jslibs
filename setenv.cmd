@echo off
IF "%BUILD%"=="" set BUILD=release
set PATH=%CD%\%BUILD%;%PATH%
echo jslibs %BUILD% environment set.
