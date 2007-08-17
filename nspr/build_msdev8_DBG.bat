set prevDir=%CD%
set prevPath=%PATH%

set path=C:\tools\cygwin\bin;%CD%

	rem extract nsinstall.exe in the /nspr directory

call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /XP32 /DEBUG
call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86

  rem  --enable-debug-rtl      Use the MSVC debug runtime library
  rem --enable-optimize --disable-debug

md win32
cd win32
	
make distclean

	rem --enable-win32-target=WIN95 --enable-debug-rtl  --with-dist-prefix=..
	
sh ../nsprpub/configure --enable-win32-target=WIN95
	rem --enable-debug-rtl

make clean all


cd ..

set PATH=%prevPath%
cd /D %prevDir%

pause