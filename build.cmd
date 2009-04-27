
set MOZ_BUILD_SCRIPT=D:\TOOLS\mozilla-build\start-msvc8.bat

rem ==========================================================================
for %%A in ("%CD%") do set _CD_SHORT=%%~sfA
echo cd "%_CD_SHORT%";sh build.sh | call %MOZ_BUILD_SCRIPT%
pause
