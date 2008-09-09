@echo off
IF "%BUILD%"=="" set BUILD=release
echo building %BUILD% version.
IF NOT "%jslibsEnvSet%"=="true" call .\envbuild.cmd

set logFile="%CD%\buildlog.txt"
set buildDir="%CD%\%BUILD%"

( date /T && time /T && set ) > %logFile%
mkdir %buildDir%

pushd .\libs\js
call buildJS.cmd >> %logFile% 2>&1
popd

pushd .\libs\nspr
call buildNSPR.cmd >> %logFile% 2>&1
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
		)
	)
)

echo Build done.
goto end

:error
echo Build failed!

:end
pause
