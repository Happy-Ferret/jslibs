IF "%BUILD%"=="" set BUILD=release
IF "%BUILD_METHOD%"=="" set BUILD_METHOD=rebuild

echo %BUILD_METHOD% SpiderMonkey "%BUILD%" version ...
call ..\..\envbuild.cmd

set _DESTINATION_DIR=..\..\%BUILD%\
set _DESTINATION_FILES=js32.dll js32.lib

pushd %_DESTINATION_DIR%
del %_DESTINATION_FILES%
popd
md %_DESTINATION_DIR%

IF "%BUILD_METHOD%"=="rebuild" (
	set _MAKE_OPTIONS=clean all
) else (
	set _MAKE_OPTIONS=all
)

IF "%BUILD%"=="release" (
	set _MAKE_OPTIONS=%_MAKE_OPTIONS% BUILD_OPT=1
) else (
	set _MAKE_OPTIONS=%_MAKE_OPTIONS% BUILD_IDG=1
)

copy jsconfig.h src

pushd .\src
make -f Makefile.ref %_MAKE_OPTIONS%
IF "%BUILD%"=="release" ( cd *OPT.OBJ ) ELSE ( cd *DBG.OBJ )
cp %_DESTINATION_FILES% ..\..\%_DESTINATION_DIR%
popd



rem ==========================================================================
rem Note: /MT : Multithreaded static, /MTd : Multithreaded static debug, /MD : Multithreaded DLL, /MDd : Multithreaded DLL debug.

rem /D_NSPR_BUILD_
rem set OTHER_LIBS=wsock32.lib ../../nspr/lib/libnspr4_s.lib
rem  JS_THREADSAFE=1 JS_DIST=../../nspr

rem make SM crash at GC:  /DJS_DUMP_PROPTREE_STATS
rem /MDd
rem /D_NSPR_BUILD_
rem set OTHER_LIBS=wsock32.lib ../../nspr/lib/libnspr4_s.lib /NODEFAULTLIB:libnspr4
rem set OTHER_LIBS=/NODEFAULTLIB:../../nspr/win32/dist/lib/libnspr4.lib
rem  JS_THREADSAFE=1 JS_DIST=../../nspr

rem flags for release version: /O2 /Ox /Oi /Ot /Oy /GL /GS- /arch:SSE
rem SM flags: /DJS_HASHMETER /DJS_GC_ZEAL
