set prevDir=%CD%
set prevPath=%PATH%

set path=C:\tools\cygwin\bin

call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /XP32 /RETAIL
call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86


rem XCFLAGS=/DJS_HAS_XDR_FREEZE_THAW=1

cd .\src
set JS_DIST=../dist


rem BUILD_OPT=1 produce a Release version, and ommiting this will produce a Debug version
rem XCFLAGS=/MT allows to produce a .exe with staticaly linked msvcrt ( then msvcr80.dll is not needed )
rem add /LTCG to the link command line to improve linker performance

set XCFLAGS=/MT /O2 /Ox /Oi /Ot /Oy /GL /GS- /arch:SSE
make -f Makefile.ref clean all BUILD_OPT=1
rem JS_THREADSAFE=1 

rem cat: ../../dist/WINNT5.1_OPT.OBJ/nspr/Version

set PATH=%prevPath%
cd %prevDir%
