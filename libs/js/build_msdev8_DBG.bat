set prevPath=%PATH%

rem ---------- configuration -------------------------------------------------

call "C:\Program Files\Microsoft SDKs\Windows\v6.1\bin\SetEnv.Cmd" /Debug /x86 /xp
set path=C:\tools\cygwin\bin;%path%
call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86


rem --------------------------------------------------------------------------

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
