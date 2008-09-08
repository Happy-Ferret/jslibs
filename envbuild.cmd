rem --- Visual Studio path ---
set VSINSTALLDIR=C:\Program Files\Microsoft Visual Studio 8

rem --- Windows Platform SDK path ---
set MSSdk=C:\Program Files\Microsoft Platform SDK
rem -or- set MSSdk=C:\Program Files\Microsoft SDKs\Windows\v6.0A

rem --- DirectX path ---
set DirectXPath=C:\Program Files\Microsoft DirectX SDK (March 2008)

rem --- cygwin path ---
set cygwinInstallDir=C:\tools\cygwin


rem ==========================================================================

IF "%BUILD%"=="release" set NODEBUG=1
IF "%BUILD%"=="release" set DEBUGMSG=RETAIL

set CPU=i386
set TARGETOS=WINNT
set APPVER=5.01

rem The following line is needed for OpenAL compilation (.\OpenAL32.rc(10) : fatal error RC1015: cannot open include file 'afxres.h'.)
set INCLUDE=%MSSdk%\Include\mfc;%MSSdk%\Include\atl;%INCLUDE%

set INCLUDE=%DirectXPath%\Include;%INCLUDE%
set LIB=%DirectXPath%\Lib\x86;%LIB%

set PATH=%cygwinInstallDir%\bin;%PATH%

set PATH=%VSINSTALLDIR%\VC\BIN;%VSINSTALLDIR%\VC\VCPackages;%VSINSTALLDIR%\Common7\IDE;%PATH%
set INCLUDE=%VSINSTALLDIR%\VC\INCLUDE;%INCLUDE%
set LIB=%VSINSTALLDIR%\VC\LIB;%LIB%

set PATH=%MSSdk%\Bin;%PATH%
set INCLUDE=%MSSdk%\Include;%INCLUDE%
set LIB=%MSSdk%\Lib;%LIB%

set jslibsEnvSet=true
