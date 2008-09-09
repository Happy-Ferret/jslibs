IF "%BUILD%"=="" set BUILD=release
echo building SpiderMonkey "%BUILD%" version ...
IF NOT "%jslibsEnvSet%"=="true" call ..\..\envbuild.cmd

set destinationPath=..\..\%BUILD%\
set destinationFiles=js32.dll js32.lib


pushd %destinationPath%
del %destinationFiles%
popd
md %destinationPath%
IF "%BUILD%"=="release" (
	set XCFLAGS=/DWINVER=0x500 /D_WIN32_WINNT=0x500 /O2 /Ox /Oi /Ot /Oy /GL /GS- /arch:SSE /D_CRT_SECURE_NO_DEPRECATE=1 /MD
	set makeOptions=BUILD_OPT=1
) else (
	set XCFLAGS=/DWINVER=0x500 /D_WIN32_WINNT=0x500 /D_CRT_SECURE_NO_DEPRECATE=1 /MDd   /DJS_HASHMETER /DJS_GC_ZEAL
	set makeOptions=
)
copy jsconfig.h src


pushd .\src
make -f Makefile.ref clean all %makeOptions%
IF "%BUILD%"=="release" ( cd *OPT.OBJ ) ELSE ( cd *DBG.OBJ )
cp %destinationFiles% ..\..\%destinationPath%
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
