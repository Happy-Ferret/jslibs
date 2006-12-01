set prevDir=%CD%
set prevPath=%PATH%

set path=C:\tools\cygwin\bin

call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /XP32 /DEBUG
call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86

cd .\src
set JS_DIST=../dist


rem XCFLAGS=/DJS_HAS_XDR_FREEZE_THAW=1
rem BUILD_OPT=1 produce a Release version, and ommiting this will produce a Debug version
rem XCFLAGS=/MT allows to produce a .exe with staticaly linked msvcrt ( then msvcr80.dll is not needed )
rem set USE_MSVC=1

set XCFLAGS=/MT
make -f Makefile.ref clean all
rem JS_THREADSAFE=1

set PATH=%prevPath%
cd %prevDir%
