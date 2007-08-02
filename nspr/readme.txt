Download:
	ftp://ftp.mozilla.org/pub/mozilla.org/nspr/releases/
	ftp://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.6.1/WINNT5.0_OPT.OBJ
	( do not extract the source with winzip, use instead: tar xfz ... )

CVS:
	cvs -d :pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot login
	cvs -d :pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot co -d src mozilla/nsprpub

Directory structure:
	.
	..
	include/nspr
	lib
	README.TXT

API reference:
	http://developer.mozilla.org/en/docs/NSPR_API_Reference

Build procedure:
	http://www.mozilla.org/projects/nspr/eng-process/build.html
	http://developer.mozilla.org/en/docs/NSPR_build_instructions


Windows compilation (debug version):
	set path=C:\tools\cygwin\bin
	call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /XP32 /DEBUG
	call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86
	cd nsprpub
	sh configure --enable-debug-rtl
	cd pr/src
	make clean all


Linux compilation:
	./configure --disable-debug --enable-strip --enable-nspr-threads --enable-optimize=-O3
	make

Misc:
	static link: define _NSPR_BUILD_, link  with  ...\nspr-4.6\lib\libnspr4_s.lib
	tests: http://developer.mozilla.org/en/docs/Running_NSPR_tests
