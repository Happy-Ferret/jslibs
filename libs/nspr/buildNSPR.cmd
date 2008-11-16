IF "%BUILD%"=="" set BUILD=release
IF "%BUILD_METHOD%"=="" set BUILD_METHOD=rebuild

echo configuring NSPR ...
call ..\..\envbuild.cmd

set _DEST_PATH=..\..\%BUILD%\
set _DEST_FILES=nspr4.dll nspr4.lib
set _BUILD_DIR=win32_%BUILD%

pushd %_DEST_PATH%
del %_DEST_FILES%
popd

if NOT EXIST nsinstall.exe echo FATAL ERROR - nsinstall.exe MUST be in this directory (jslibs/nspr/) && exit

set _PREV_PATH=%PATH%
set PATH=%CD%;%PATH%

for %%A in ("%CD%") do set _CD_SHORT=%%~spnxA

md %_BUILD_DIR%

pushd %_CD_SHORT%\%_BUILD_DIR%

IF "%BUILD_METHOD%"=="rebuild" (
	rmdir /S /Q .
	set _MAKE_OPTIONS=clean all
) ELSE (
	set _MAKE_OPTIONS=all
)

REM using --disable-debug option break the compilation !
IF "%BUILD%"=="release" (
	set _CONFIG_OPTIONS=--enable-win32-target=WIN95
) ELSE (
	set _CONFIG_OPTIONS=--enable-win32-target=WIN95
)

IF NOT EXIST Makefile (
	sh ../src/configure %_CONFIG_OPTIONS%
)

IF "%BUILD%"=="release" (
	set XCFLAGS=/MD
) else (
	set XCFLAGS=/MDd
)
echo building NSPR ...
make %_MAKE_OPTIONS%
popd

echo copying NSPR ...
pushd %_BUILD_DIR%\dist\lib\
for %%a in (%_DEST_FILES%) do copy %%a ..\..\..\%_DEST_PATH%
popd

set PATH=%_PREV_PATH%

rem ==========================================================================
rem Note: /MT : Multithreaded static, /MTd : Multithreaded static debug, /MD : Multithreaded DLL, /MDd : Multithreaded DLL debug.
rem  --enable-debug-rtl      Use the MSVC debug runtime library
rem --enable-optimize --disable-debug
rem --enable-win32-target=WIN95 --enable-debug-rtl  --with-dist-prefix=..
