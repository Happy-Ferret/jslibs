set prevDir=%CD%
set prevPath=%PATH%

call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /XP32 /RETAIL
call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86

set path=%path%;C:\tools\cygwin\bin

rem XCFLAGS=/DJS_HAS_XDR_FREEZE_THAW=1

cd .\src

rem BUILD_OPT=1 produce a Release version, and ommiting this will produce a Debug version
rem XCFLAGS=/MT allows to produce a .exe with staticaly linked msvcrt ( then msvcr80.dll is not needed )

make -f Makefile.ref clean all BUILD_OPT=1 XCFLAGS=/MT

rem cat: ../../dist/WINNT5.1_OPT.OBJ/nspr/Version

set PATH=%prevPath%
cd %prevDir%
