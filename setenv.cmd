IF "%BUILD%"=="" set BUILD=release

IF "%BUILD%"=="release" set PATH=%CD%\js\src\WINNT5.1_OPT.OBJ;%PATH%
IF "%BUILD%"=="debug" set PATH=%CD%\js\src\WINNT5.1_DBG.OBJ;%PATH%

set PATH=%CD%\jshost\%BUILD%;%CD%\jswinhost\%BUILD%;%CD%\jslang\%BUILD%;%CD%\jsstd\%BUILD%;%CD%\jsio\%BUILD%;%CD%\nspr\win32\dist\lib;%CD%\jssqlite\%BUILD%;%CD%\jsobjex\%BUILD%;%CD%\jsz\%BUILD%;%CD%\jscrypt\%BUILD%;%CD%\jswinshell\%BUILD%;%CD%\jstest\%BUILD%;%CD%\jsdebug\%BUILD%;%CD%\jsimage\%BUILD%;%PATH%
