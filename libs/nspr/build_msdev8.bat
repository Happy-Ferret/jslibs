set prevDir=%CD%
set prevPath=%PATH%
set path=C:\tools\cygwin\bin;%CD%;%path%

call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /XP32 /DEBUG
call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86

	rem  --enable-debug-rtl      Use the MSVC debug runtime library
	rem --enable-optimize --disable-debug
	rem --enable-win32-target=WIN95 --enable-debug-rtl  --with-dist-prefix=..

cd /D x:\ && echo the drive X: must be available to build nspr, else modify build_msdev8.bat && exit

subst x: "%CD%"
pushd x:

del /S /Q .\win32
md win32

if NOT EXIST nsinstall.exe echo FATAL ERROR - nsinstall.exe MUST be in this directory (jslibs/nspr)

pushd win32
sh ../src/configure --enable-win32-target=WIN95
set XCFLAGS=/MT
make clean all
popd

popd

subst x: /D

set PATH=%prevPath%
cd /D %prevDir%

