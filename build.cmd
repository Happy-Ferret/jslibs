set MOZ_BUILD_SCRIPT=C:\TOOLS\mozilla-build\start-shell-msvc2013.bat

rem ==========================================================================
for %%A in ("%CD%") do set _CD_SHORT=%%~sfA
set PREV_USERPROFILE=%USERPROFILE%
set USERPROFILE=%_CD_SHORT%
rem set EXCLUDE=jstask.sln jsfastcgi.sln jsffi.sln jsvst.sln jstask.sln jstemplate.sln jsapi-tests.sln jsTest.sln
echo build.sh | %MOZ_BUILD_SCRIPT%
