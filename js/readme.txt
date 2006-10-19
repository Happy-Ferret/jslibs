download:
=========
	cvs -d :pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot login
	cvs -d :pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot co -d src mozilla/js/src

modify:
=======
	./src/jsconfig.h: enable JS_HAS_XDR_FREEZE_THAW in the last JS_VERSION

directory structure:
====================
	.
	..
	src
	readme.txt

help:
=====
	http://developer.mozilla.org/en/docs/Introduction_to_the_JavaScript_shell
	
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

"c:\Program Files\Microsoft Visual Studio\VC98\Bin\VCVARS32.BAT"
  or
"c:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat"

cigwin.bat
cd /cygdrive/p/dev/spidermonkey/mozilla/js/src
make -f Makefile.ref


compilation with gmake under windows ( Makefile.ref )
-----------------------------------------------------

 call "c:\Program Files\Microsoft Visual Studio\VC98\Bin\VCVARS32.BAT"
 set path=%path%;C:\cygwin\bin
 make -f Makefile.ref clean all BUILD_OPT=1 

