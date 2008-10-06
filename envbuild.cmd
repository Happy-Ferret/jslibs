IF "JSLIBS_BUILD_ENV_IS_SET"=="true" GOTO END


REM --- Visual Studio path ---
REM set VSINSTALLDIR=C:\Program Files\Microsoft Visual Studio 9.0
set VSINSTALLDIR=C:\Program Files\Microsoft Visual Studio 8


REM --- Windows Platform SDK path ---
set MSSdk=C:\Program Files\Microsoft Platform SDK
REM set MSSdk=C:\Program Files\Microsoft SDKs\Windows\v6.0A

REM --- DirectX path ---
set DirectXPath=C:\Program Files\Microsoft DirectX SDK (March 2008)

REM --- GNU tools path ---

rem set gnuTools=C:\TOOLS\mozilla-build\msys\bin
rem set gnuTools=C:\tools\msys\bin
set gnuTools=C:\tools\cygwin\bin



REM ==========================================================================

IF "%BUILD%"=="release" set NODEBUG=1
IF "%BUILD%"=="release" set DEBUGMSG=RETAIL

set CPU=i386
set TARGETOS=WINNT
set APPVER=5.01

REM The following line is needed for OpenAL compilation (.\OpenAL32.rc(10) : fatal error RC1015: cannot open include file 'afxres.h'.)
set INCLUDE=%MSSdk%\Include\mfc;%MSSdk%\Include\atl;%INCLUDE%

set INCLUDE=%DirectXPath%\Include;%INCLUDE%
set LIB=%DirectXPath%\Lib\x86;%LIB%

set PATH=%gnuTools%;%PATH%

set PATH=%VSINSTALLDIR%\VC\BIN;%VSINSTALLDIR%\VC\VCPackages;%VSINSTALLDIR%\Common7\IDE;%PATH%
set INCLUDE=%VSINSTALLDIR%\VC\INCLUDE;%INCLUDE%
set LIB=%VSINSTALLDIR%\VC\LIB;%LIB%

set PATH=%MSSdk%\Bin;%PATH%
set INCLUDE=%MSSdk%\Include;%INCLUDE%
set LIB=%MSSdk%\Lib;%LIB%


set CC=cl
set CXX=cl
set LD=link
rem set AR=lib -NOLOGO -OUT:"$@"
set OBJ_SUFFIX=obj
set LIB_SUFFIX=lib
set DLL_SUFFIX=dll



set JSLIBS_BUILD_ENV_IS_SET=true
:END
