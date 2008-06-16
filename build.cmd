set visualStudioPath=C:\Program Files\Microsoft Visual Studio 8
set platformSDKPath=C:\Program Files\Microsoft Platform SDK
set DirectXPath=C:\Program Files\Microsoft DirectX SDK (March 2008)

rem --------------------------------------------------------------------------
@echo off

IF "%BUILD%"=="" set BUILD=release

set prevDir=%CD%
set prevPath=%PATH%

Set Lib=%DirectXPath%\Lib\x86;%Lib%
Set Include=%DirectXPath%\Include;%Include%

call "%platformSDKPath%\SetEnv.Cmd" /XP32 /RETAIL
rem The following line is needed for OpenAL compilation (.\OpenAL32.rc(10) : fatal error RC1015: cannot open include file 'afxres.h'.)
set Include=%Include%;%MSSdk%\Include\mfc;%MSSdk%\Include\atl

call "%visualStudioPath%\VC\vcvarsall.bat" x86

date /T > build.log
set >> build.log


pushd .\libs\js
IF "%BUILD%" == "release" (
	call build_msdev8_OPT.bat >> build.log
) ELSE (
	call build_msdev8_DBG.bat >> build.log
)
popd

pushd .\libs\nspr
call build_msdev8.bat >> build.log
popd




md .\%BUILD%


copy .\libs\js\src\WINNT5.1_OPT.OBJ\*.dll .\%BUILD%
copy .\libs\nspr\win32\dist\lib\nspr4.dll .\%BUILD%

echo building %BUILD% version.

for /D %%f in (src\js*) do (

	for %%g in ("%%f\*.sln") do (
		
		echo building %%g ...
		echo ******************************************************************************* >> build.log
		echo ******************************************************************************* >> build.log
		rem VCBUILD Options: http://msdn2.microsoft.com/en-us/library/cz553aa1(VS.80).aspx
		"%visualStudioPath%\VC\vcpackages\vcbuild" /useenv /rebuild %%g "%BUILD%|WIN32" >> build.log
		if ERRORLEVEL 1 (
		
			echo ... failed !
rem		if ERRORLEVEL 1 goto error
		) ELSE (

rem			echo ... ok.

			pushd %%~dg%%~pg%BUILD%
			copy %%~nf.dll ..\..\..\%BUILD% 1>nul
			copy %%~nf.exe ..\..\..\%BUILD% 1>nul
			popd
		)
	)
)

echo Build done.
goto end

:error
echo Build failed!

:end
pause
