@echo off
IF "%BUILD%"=="" set BUILD=release
IF "%BUILD_METHOD%"=="" set BUILD_METHOD=rebuild

echo building NSPR ...
call ..\..\envbuild.cmd

set _DEST_PATH=..\..\%BUILD%\
set _DEST_FILES=nspr4.dll nspr4.lib

set _BUILD_DIR=win32_%BUILD%

pushd %_DEST_PATH%
del %_DEST_FILES%
popd

REM the tmpDrive is used to avoid spaces in filenames
set tmpDrive=x:
subst %tmpDrive% /D

cd /D %tmpDrive% && echo the drive %tmpDrive% must be available to build nspr, else modify build_msdev8.bat && exit
if NOT EXIST nsinstall.exe echo FATAL ERROR - nsinstall.exe MUST be in this directory (jslibs/nspr) && exit

set _PREV_PATH=%PATH%
set PATH=%CD%;%PATH%

subst %tmpDrive% "%CD%"
pushd %tmpDrive%

md %_BUILD_DIR%
pushd %_BUILD_DIR%

IF "%BUILD_METHOD%"=="rebuild" (
	rmdir /S /Q .
	set _MAKE_OPTIONS=clean all
) else (
	set _MAKE_OPTIONS=all
)

IF NOT EXIST Makefile (
  sh ../src/configure --enable-win32-target=WIN95
)

IF "%BUILD%"=="release" (
	set XCFLAGS=/MD
) else (
	set XCFLAGS=/MDd
)

make %_MAKE_OPTIONS%
popd

set PATH=%_PREV_PATH%

popd
subst %tmpDrive% /D

pushd %_BUILD_DIR%\dist\lib\
cp %_DEST_FILES% ..\..\..\%_DEST_PATH%
popd


rem ==========================================================================
rem Note: /MT : Multithreaded static, /MTd : Multithreaded static debug, /MD : Multithreaded DLL, /MDd : Multithreaded DLL debug.
rem  --enable-debug-rtl      Use the MSVC debug runtime library
rem --enable-optimize --disable-debug
rem --enable-win32-target=WIN95 --enable-debug-rtl  --with-dist-prefix=..
