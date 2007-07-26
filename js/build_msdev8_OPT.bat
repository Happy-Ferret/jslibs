set prevDir=%CD%
set prevPath=%PATH%

set path=C:\tools\cygwin\bin

call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /XP32 /RETAIL
call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86

cd .\src

set XCFLAGS=/O2 /Ox /Oi /Ot /Oy /GL /GS- /arch:SSE /D_CRT_SECURE_NO_DEPRECATE=1
make -f Makefile.ref clean all BUILD_OPT=1 JS_THREADSAFE=1 JS_DIST=../../nspr

set PATH=%prevPath%
cd /D %prevDir%
