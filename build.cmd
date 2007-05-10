set prevDir=%CD%
set prevPath=%PATH%

set visualStudioPath=C:\Program Files\Microsoft Visual Studio 8
set platformSDKPath=C:\Program Files\Microsoft Platform SDK

rem --------------------------------------------------------------------------

call "%platformSDKPath%\SetEnv.Cmd"
call "%visualStudioPath%\VC\vcvarsall.bat"

del build.log

for /D %%f in (*) do (

	for %%g in (%%f\*.sln) do (
		
		echo building %%g
		rem VCBUILD Options: http://msdn2.microsoft.com/en-us/library/cz553aa1(VS.80).aspx
		"%visualStudioPath%\VC\vcpackages\vcbuild" /useenv /rebuild %%g "RELEASE|WIN32" >> build.log
		if ERRORLEVEL 1 goto error
		pushd %%~dg%%~pgrelease
		copy *.dll ..\..
		copy *.exe ..\..
		popd
	)
)

echo Build done.
goto :EOF
:error
echo Build failed!

pause

