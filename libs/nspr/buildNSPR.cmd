@echo off
IF "%BUILD%"=="" set BUILD=release
echo building NSPR ...
IF NOT "%jslibsEnvSet%"=="true" call ..\..\envbuild.cmd

set destinationPath=..\..\%BUILD%\
set destinationFiles=nspr4.dll nspr4.lib



pushd %destinationPath%
del %destinationFiles%
popd

REM the tmpDrive is used to avoid spaces in filenames
set tmpDrive=x:

cd /D %tmpDrive% && echo the drive %tmpDrive% must be available to build nspr, else modify build_msdev8.bat && exit
if NOT EXIST nsinstall.exe echo FATAL ERROR - nsinstall.exe MUST be in this directory (jslibs/nspr) && exit

set prevPath=%PATH%
set PATH=%CD%;%PATH%

subst %tmpDrive% "%CD%"
pushd %tmpDrive%

del /S /Q .\win32
md win32

pushd win32
sh ../src/configure --enable-win32-target=WIN95

IF "%BUILD%"=="release" (
	set XCFLAGS=/MD
) else (
	set XCFLAGS=/MDd
)

make clean all
popd

set PATH=%prevPath%

popd
subst %tmpDrive% /D

pushd win32\dist\lib\
cp %destinationFiles% ..\..\..\%destinationPath%
popd


rem ==========================================================================
rem Note: /MT : Multithreaded static, /MTd : Multithreaded static debug, /MD : Multithreaded DLL, /MDd : Multithreaded DLL debug.
rem  --enable-debug-rtl      Use the MSVC debug runtime library
rem --enable-optimize --disable-debug
rem --enable-win32-target=WIN95 --enable-debug-rtl  --with-dist-prefix=..
