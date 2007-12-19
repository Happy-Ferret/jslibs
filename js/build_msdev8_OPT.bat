set prevDir=%CD%
set prevPath=%PATH%

set path=C:\tools\cygwin\bin

call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /XP32 /RETAIL
call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86

copy jsconfig.h src

cd .\src

	rem /MT  : Multithreaded static
	rem /MTd : Multithreaded static debug
	rem /MD  : Multithreaded DLL
	rem /MDd : Multithreaded DLL debug
set XCFLAGS=/DWINVER=0x500 /D_WIN32_WINNT=0x500 /O2 /Ox /Oi /Ot /Oy /GL /GS- /arch:SSE /D_CRT_SECURE_NO_DEPRECATE=1 /MD
	rem /D_NSPR_BUILD_
	rem set OTHER_LIBS=wsock32.lib ../../nspr/lib/libnspr4_s.lib
make -f Makefile.ref clean all BUILD_OPT=1
rem  JS_THREADSAFE=1 JS_DIST=../../nspr 

set PATH=%prevPath%
cd /D %prevDir%
