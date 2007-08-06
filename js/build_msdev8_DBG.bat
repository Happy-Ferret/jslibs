set prevDir=%CD%
set prevPath=%PATH%

set path=C:\tools\cygwin\bin

call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /XP32 /DEBUG
call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86

cd .\src

	rem /MT  : Multithreaded static
	rem /MTd : Multithreaded static debug
	rem /MD  : Multithreaded DLL
	rem /MDd : Multithreaded DLL debug
set XCFLAGS=/D_CRT_SECURE_NO_DEPRECATE=1 /MDd

	rem set OTHER_LIBS=../../nspr/win32/dist/lib/nspr4.lib /NODEFAULTLIB:libnspr4.lib

	rem Linker Options: http://msdn2.microsoft.com/en-us/library/y0zzbyt4(VS.80).aspx
	rem TEMPORARY ( may crash with Date object ) :
	rem set OTHER_LIBS=/FORCE:UNRESOLVED

make -f Makefile.ref clean all   JS_THREADSAFE=1 JS_DIST=../../nspr
	rem JS_DIST=../../nspr/win32/dist

set PATH=%prevPath%
cd /D %prevDir%
