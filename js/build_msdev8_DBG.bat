set prevDir=%CD%
set prevPath=%PATH%

set path=C:\tools\cygwin\bin

call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /XP32 /DEBUG
call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86

copy jsconfig.h src

cd .\src

	rem /MT  : Multithreaded static
	rem /MTd : Multithreaded static debug
	rem /MD  : Multithreaded DLL
	rem /MDd : Multithreaded DLL debug
set XCFLAGS=/DWINVER=0x500 /D_WIN32_WINNT=0x500 /D_CRT_SECURE_NO_DEPRECATE=1 /MDd /DJS_HASHMETER
	rem /MDd
	rem /D_NSPR_BUILD_
	rem set OTHER_LIBS=wsock32.lib ../../nspr/lib/libnspr4_s.lib /NODEFAULTLIB:libnspr4
	
	rem set OTHER_LIBS=/NODEFAULTLIB:../../nspr/win32/dist/lib/libnspr4.lib

set INCLUDES=-I"D:\Franck\Mes documents\DEV\my_projects\jslibs\js"

make -f Makefile.ref clean all
rem  JS_THREADSAFE=1 JS_DIST=../../nspr

set PATH=%prevPath%
cd /D %prevDir%

