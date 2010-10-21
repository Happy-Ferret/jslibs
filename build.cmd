set MOZ_BUILD_SCRIPT=C:\TOOLS\mozilla-build\start-msvc9.bat

rem ==========================================================================
for %%A in ("%CD%") do set _CD_SHORT=%%~sfA
echo cd "%_CD_SHORT%";sh build.sh "%1" | call %MOZ_BUILD_SCRIPT%

pause
