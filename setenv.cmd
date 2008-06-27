echo off

IF "%BUILD%"=="" set BUILD=release

IF "%BUILD%"=="release" set PATH=%CD%\libs\js\src\WINNT5.1_OPT.OBJ;%PATH%
IF "%BUILD%"=="debug" set PATH=%CD%\libs\js\src\WINNT5.1_DBG.OBJ;%PATH%

set PATH=%PATH%;%CD%\src\jshost\%BUILD%;%CD%\src\jswinhost\%BUILD%;%CD%\src\jslang\%BUILD%;%CD%\src\jsstd\%BUILD%;%CD%\src\jsio\%BUILD%;%CD%\libs\nspr\win32\dist\lib;%CD%\src\jssqlite\%BUILD%;%CD%\src\jsobjex\%BUILD%;%CD%\src\jsz\%BUILD%;%CD%\src\jscrypt\%BUILD%;%CD%\src\jswinshell\%BUILD%;%CD%\src\jsdebug\%BUILD%;%CD%\src\jsimage\%BUILD%;%CD%\src\jssound\%BUILD%

echo jslibs %BUILD% environment set.
