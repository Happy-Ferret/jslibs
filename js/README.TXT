download:
=========
	cvs -d :pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot login
	cvs -d :pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot co -d src mozilla/js/src

modify:
=======
	./src/jsconfig.h:
	  disable JS_HAS_STR_HTML_HELPERS
	  enable JS_HAS_XDR
	  enable JS_HAS_XDR_FREEZE_THAW
	  enable JS_HAS_SCRIPT_OBJECT
	in the last JS_VERSION

directory structure:
====================
	.
	..
	src
	readme.txt

help:
=====
	http://developer.mozilla.org/en/docs/Introduction_to_the_JavaScript_shell

	Visual C++ Linker Options: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vccore/html/_core_linker_reference.asp
	
multithreaded version:
======================
  need to copy NSPR 4+ in:
  ./dist/include/nspr
  ./dist/lib


misc:
=====
	jsxml.h & jsxml.c should be linked with spidermonkey
	msvc: need /OPT:NOICF compilation option ! (cf. http://www.mozilla.org/js/spidermonkey/release-notes/)
	JSFILE ( http://www.mozilla.org/js/js-file-object.html ); add these defines: JS_THREADSAFE=1, JS_HAS_FILE_OBJECT=1
	about JS_HAS_XDR_FREEZE_THAW : XCFLAGS=/DJS_HAS_XDR_FREEZE_THAW=1  do not work because it is overwritten by jsconfig.h content

todo:
=====
	try JS_THREADSAFE=1, JS_HAS_FILE_OBJECT=1
	
notes:
======


compilation with namke
----------------------

"c:\Program Files\Microsoft Visual Studio\VC98\Bin\VCVARS32.BAT"
NMAKE /f "js.mak" CFG="jsshell - Win32 Debug"

=> unfortunately "js.mak" is not up to date


compilation with gmake ( Makefile.ref )
---------------------------------------

cf. http://www.mozilla.org/classic/build/classic-win.html

tools: ( GNU utilities for Win32: http://unxutils.sourceforge.net/ )
  gmake.exe, uname.exe, rm.exe, cp.exe
  
=> pb with regexpr (??)


compilation with gmake under cygwin ( Makefile.ref )
----------------------------------------------------

set path=C:\tools\cygwin\bin

call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /XP32 /DEBUG
call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86

cd .\src

rem BUILD_OPT=1 produce a Release version, and ommiting this will produce a Debug version
rem XCFLAGS=/MT allows to produce a .exe with staticaly linked msvcrt ( then msvcr80.dll is not needed )
rem set USE_MSVC=1

rem /MT
set XCFLAGS=/D_CRT_SECURE_NO_DEPRECATE=1
make -f Makefile.ref clean all JS_THREADSAFE=1 JS_DIST=../../nspr


compilation with gmake under windows ( Makefile.ref )
-----------------------------------------------------

 call "c:\Program Files\Microsoft Visual Studio\VC98\Bin\VCVARS32.BAT"
 set path=%path%;C:\cygwin\bin
 make -f Makefile.ref clean all BUILD_OPT=1 

