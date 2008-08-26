echo off

IF "%BUILD%"=="" set BUILD=release

IF "%BUILD%"=="release" set PATH=%CD%\libs\js\src\WINNT5.1_OPT.OBJ;%PATH%
IF "%BUILD%"=="debug" set PATH=%CD%\libs\js\src\WINNT5.1_DBG.OBJ;%PATH%


set PATH=%PATH%;%CD%\src\jshost\%BUILD%;
set PATH=%PATH%;%CD%\src\jswinhost\%BUILD%;
set PATH=%PATH%;%CD%\src\jslang\%BUILD%;

set PATH=%PATH%;%CD%\src\jsstd\%BUILD%;
set PATH=%PATH%;%CD%\src\jsio\%BUILD%;
set PATH=%PATH%;%CD%\libs\nspr\win32\dist\lib;
set PATH=%PATH%;%CD%\src\jssqlite\%BUILD%;
set PATH=%PATH%;%CD%\src\jsobjex\%BUILD%;
set PATH=%PATH%;%CD%\src\jsz\%BUILD%;
set PATH=%PATH%;%CD%\src\jscrypt\%BUILD%;
set PATH=%PATH%;%CD%\src\jswinshell\%BUILD%;
set PATH=%PATH%;%CD%\src\jsdebug\%BUILD%;
set PATH=%PATH%;%CD%\src\jsimage\%BUILD%;
set PATH=%PATH%;%CD%\src\jsgraphics\%BUILD%;
set PATH=%PATH%;%CD%\src\jssound\%BUILD%;
set PATH=%PATH%;%CD%\src\jsaudio\%BUILD%;
set PATH=%PATH%;%CD%\src\jsfont\%BUILD%;
set PATH=%PATH%;%CD%\src\jsprotex\%BUILD%;
set PATH=%PATH%;%CD%\libs\openal\src\OpenAL-Windows\OpenAL32\Release

echo jslibs %BUILD% environment set.
