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

make -f Makefile.ref clean all   JS_THREADSAFE=1 JS_DIST=../../nspr

set PATH=%prevPath%
cd /D %prevDir%
