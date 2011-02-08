set MOZ_BUILD_SCRIPT=C:\TOOLS\mozilla-build\start-msvc9.bat

rem ==========================================================================
for %%A in ("%CD%") do set _CD_SHORT=%%~sfA
set PREV_USERPROFILE=%USERPROFILE%
set USERPROFILE=%_CD_SHORT%
call %MOZ_BUILD_SCRIPT%
