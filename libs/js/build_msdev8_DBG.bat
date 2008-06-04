set prevPath=%PATH%

set path=C:\tools\cygwin\bin

call "%ProgramFiles%\Microsoft Platform SDK\SetEnv.Cmd" /XP32 /DEBUG
call "%ProgramFiles%\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86

pushd .\src

copy ..\jsconfig.h .

	rem /MT  : Multithreaded static
	rem /MTd : Multithreaded static debug
	rem /MD  : Multithreaded DLL
	rem /MDd : Multithreaded DLL debug
set XCFLAGS=/DWINVER=0x500 /D_WIN32_WINNT=0x500 /D_CRT_SECURE_NO_DEPRECATE=1 /MDd
set XCFLAGS=%XCFLAGS% /DJS_HASHMETER /DJS_GC_ZEAL

	rem make SM crash at GC:  /DJS_DUMP_PROPTREE_STATS
	rem /MDd
	rem /D_NSPR_BUILD_
	rem set OTHER_LIBS=wsock32.lib ../../nspr/lib/libnspr4_s.lib /NODEFAULTLIB:libnspr4
	rem set OTHER_LIBS=/NODEFAULTLIB:../../nspr/win32/dist/lib/libnspr4.lib

make -f Makefile.ref clean all
	rem  JS_THREADSAFE=1 JS_DIST=../../nspr

set PATH=%prevPath%

popd
