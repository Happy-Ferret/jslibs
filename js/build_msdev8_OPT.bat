set prevDir=%CD%
set prevPath=%PATH%

set path=C:\tools\cygwin\bin

call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /XP32 /RETAIL
call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86


rem XCFLAGS=/DJS_HAS_XDR_FREEZE_THAW=1

cd .\src

rem BUILD_OPT=1 produce a Release version, and ommiting this will produce a Debug version
rem XCFLAGS=/MT allows to produce a .exe with staticaly linked msvcrt ( then msvcr80.dll is not needed )

make -f Makefile.ref clean all BUILD_OPT=1 XCFLAGS="/MT /O2 /Ox /Oi /Ot /Oy /GL /GS- /arch:SSE"

rem cat: ../../dist/WINNT5.1_OPT.OBJ/nspr/Version

set PATH=%prevPath%
cd %prevDir%


 for details.'
link.exe -out:"WINNT5.1_OPT.OBJ/jskwgen.exe" kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib oldnames.lib -nologo -subsystem:console -debug -pdb:WINNT5.1_OPT.OBJ/jskwgen.pdb -machine:I386 -opt:ref -opt:noicf WINNT5.1_OPT.OBJ/jskwgen.obj
link: invalid option -- o
Try `link --help' for more information.
make[1]: *** [WINNT5.1_OPT.OBJ/jskwgen.exe] Error 1
make[1]: Leaving directory `/cygdrive/c/PERSO/projects/js/src'
make: *** [all] Error 2