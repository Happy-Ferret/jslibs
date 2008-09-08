@echo off
IF "%BUILD%"=="" set BUILD=release
echo building %BUILD% version.
IF NOT "%jslibsEnvSet%"=="true" call .\envbuild.cmd

set logFile="%CD%\buildlog.txt"
set buildDir="%CD%\%BUILD%"

( date /T && time /T && set ) > %logFile%
md %buildDir%

pushd .\libs\js
IF "%BUILD%"=="release" (
	call buildJS.cmd >> %logFile% 2>&1
	copy src\WINNT5.1_OPT.OBJ\*.dll %buildDir%
) ELSE (
	call buildJS.cmd >> %logFile% 2>&1
	copy src\WINNT5.1_OPT.OBJ\*.dll %buildDir%
)
popd

pushd .\libs\nspr
call build_msdev8.bat >> %logFile% 2>&1
copy win32\dist\lib\nspr4.dll %buildDir%
popd


echo building jslibs %BUILD% version.

for /D %%f in (src\js*) do (

	for %%g in ("%%f\*.sln") do (
		
		echo building %%g ...
		echo ******************************************************************************* >> %logFile%
		echo ******************************************************************************* >> %logFile%
		vcbuild /useenv /rebuild %%g "%BUILD%|WIN32" >> %logFile% 2>&1
		if ERRORLEVEL 1 (
		
			echo ... failed !
		) ELSE (

			pushd %%~dg%%~pg%BUILD%
			copy %%~nf.dll %buildDir% 1>nul
			copy %%~nf.exe %buildDir% 1>nul
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
