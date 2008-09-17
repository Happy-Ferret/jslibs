@echo off
IF "%BUILD%"=="" set BUILD=release
IF "%BUILD_METHOD%"=="" set BUILD_METHOD=rebuild

echo building %BUILD% version.
call .\envbuild.cmd

set _LOG_FILE="%CD%\%BUILD%\buildlog.txt"
set _BUILD_PATH="%CD%\%BUILD%"

echo %DATE% %TIME% > %_LOG_FILE%
mkdir %_BUILD_PATH%

echo building SpiderMonkey ...
pushd .\libs\js
call buildJS.cmd >> %_LOG_FILE% 2>&1
popd

echo building NSPR ...
pushd .\libs\nspr
call buildNSPR.cmd >> %_LOG_FILE% 2>&1
popd

echo copying OpenalAL redist files ...
pushd .\libs\openal
copy .\sdk\redist\*.dll %_BUILD_PATH% >> %_LOG_FILE% 2>&1
popd


echo building jslibs ...

IF "%BUILD_METHOD%"=="rebuild" (
	set _BUILD_OPT=/rebuild
) else (
	set _BUILD_OPT=
)

for /D %%f in (src\js*) do (
	for %%g in ("%%f\*.sln") do (
		
		echo building   %%g ...
		( echo. && echo. && echo. && echo. && echo. ) >> %_LOG_FILE%
		vcbuild /nohtmllog /nologo /useenv %_BUILD_OPT% %%g "%BUILD%|WIN32" >> %_LOG_FILE% 2>&1
		if ERRORLEVEL 1 echo ... failed !
	)
)

echo Build done.
pause
