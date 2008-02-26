set visualStudioPath=C:\Program Files\Microsoft Visual Studio 8
set platformSDKPath=C:\Program Files\Microsoft Platform SDK

rem --------------------------------------------------------------------------

set prevDir=%CD%
set prevPath=%PATH%

call "%platformSDKPath%\SetEnv.Cmd" /XP32 /RETAIL
call "%visualStudioPath%\VC\vcvarsall.bat" x86

del build.log

md .\opt

for /D %%f in (js*) do (

	for %%g in (%%f\*.sln) do (
		
		echo building %%g ...
		rem VCBUILD Options: http://msdn2.microsoft.com/en-us/library/cz553aa1(VS.80).aspx
		"%visualStudioPath%\VC\vcpackages\vcbuild" /useenv /rebuild %%g "RELEASE|WIN32" >> build.log
		if ERRORLEVEL 1 (
		
			echo ... failed.
rem		if ERRORLEVEL 1 goto error
		) ELSE (

			pushd %%~dg%%~pgrelease
			copy %%f.dll ..\..\opt
			copy %%f.exe ..\..\opt
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
