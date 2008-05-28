set prevDir=%CD%
set prevPath=%PATH%
set path=C:\tools\cygwin\bin;%CD%;%path%

call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /XP32 /DEBUG
call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86

subst x: "%CD%"
pushd x:

del /S /Q .\win32
md win32

pushd win32
sh ../src/configure
popd

popd

subst x: /D

set PATH=%prevPath%
cd /D %prevDir%

pause
