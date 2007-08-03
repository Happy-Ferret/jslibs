Download:
	ftp://ftp.mozilla.org/pub/mozilla.org/nspr/releases/
	ftp://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.6.1/WINNT5.0_OPT.OBJ
	( do not extract the source with winzip, use instead: tar xfz ... )

CVS:
	cvs -d :pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot login
	cvs -d :pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot co -d src mozilla/nsprpub

NSPR Release Notes:
	http://www.mozilla.org/projects/nspr/release-notes/

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

	need nsinstall ( http://www.mozilla.org/build/win32-nmake.html#ss2.1 )
	nsinstall.exe from cvs: :pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot/buildtools/windows/bin/x86/nsinstall.exe

Windows compilation (debug version):
	set path=C:\tools\cygwin\bin
	call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /XP32 /DEBUG
	call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86
	cd nsprpub
	sh configure --enable-win32-target=WIN95 --enable-debug-rtl
	cd pr/src
	make clean all


Linux compilation:
	./configure --disable-debug --enable-strip --enable-nspr-threads --enable-optimize=-O3
	make

NSPR static library (libnspr4_s.lib):
	It is not recomended to use it ( cf. http://www.nabble.com/Static-linking-causing-unresolved-externals-t629691.html )
	use -D_NSPR_BUILD_

Misc:
	tests: http://developer.mozilla.org/en/docs/Running_NSPR_tests

	Wan-Teh:
		By the way, NSPR has two implementations for Windows.  For
		historical reasons, the default one (called "WINNT") is not appropriate
		for general use.  I suggest that you pass --enable-win32-target=WIN95
		to 'configure'.  Don't be turned off by the name "WIN95".  It is a historical
		name.  It really means generic WIN32.


Configure options:

  --with-dist-prefix=DIST_PREFIX
                          place build files in DIST_PREFIX [dist]
  --with-dist-bindir=DIR  build execuatables in DIR [DIST_PREFIX/bin]
  --with-dist-includedir=DIR
                          build include files in DIR [DIST_PREFIX/include/nspr]
  --with-dist-libdir=DIR  build library files in DIR [DIST_PREFIX/lib]
  --with-mozilla          Compile NSPR with Mozilla support
  --enable-optimize(=val) Enable code optimizations (val, ie. -O2) 
  --disable-debug         Do not compile in debugging symbols
  --enable-debug(=val)    Enable debugging (debug flags val)
  --enable-win32-target=$t
                          Specify win32 flavor. (WIN95 or WINNT)
  --enable-debug-rtl      Use the MSVC debug runtime library
  --enable-n32            Enable n32 ABI support (IRIX only)
  --enable-64bit          Enable 64-bit support (on certain platforms)
  --enable-mdupdate       Enable use of certain compilers' mdupdate feature
  --with-macos-sdk=dir    Location of platform SDK to use (Mac OS X only)
  --enable-macos-target=VER
                          Set the minimum MacOS version needed at runtime
                          [10.1 for ppc, 10.4 for x86]
  --enable-strip          Enable stripping of shared libs and programs
  --with-pthreads         Use system pthreads library as thread subsystem
  --enable-user-pthreads  Build using userland pthreads
  --enable-nspr-threads   Build using classic nspr threads
  --with-bthreads         Use system bthreads library as thread subsystem
                          (BeOS only)
  --with-native-threads   Use native system threads as thread subsystem
                          (Solaris only)
  --enable-cplus          Enable some c++ api routines
  --enable-ipv6           Compile ipv6 support
  --enable-boehm          Enable the Boehm Garbage Collector
