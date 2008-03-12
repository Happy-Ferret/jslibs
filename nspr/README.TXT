svn:externals src http://libset.googlecode.com/svn/trunk/nspr/mozilla/nsprpub

Download:
	ftp://ftp.mozilla.org/pub/mozilla.org/nspr/releases/
	ftp://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.6.1/WINNT5.0_OPT.OBJ
	( do NOT extract the source with winzip, but use: tar xfz nspr-X.Y.Z.tar.gz )

CVS:
	cvs -d :pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot login
	cvs -d :pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot co -d nsprpub mozilla/nsprpub


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


Important note about win32-target:

	"By the way, NSPR has two implementations for Windows.  For
	historical reasons, the default one (called "WINNT") is not appropriate
	for general use.  I suggest that you pass --enable-win32-target=WIN95
	to 'configure'.  Don't be turned off by the name "WIN95".  It is a historical
	name.  It really means generic WIN32."
	Wan-Teh

	"The WIN95 target is better maintained.  For lack of time, some of the
	new features in the past four or five years were only added to the WIN95
	target.
	It is unfortunate that NSPR has two implementations on Windows
	and the obvious, better-sounding name (WINNT) is used for the
	non-default target.  This has caused a lot of confusions."
	Wan-Teh
	(source: mozilla.dev.tech.nspr@news.mozilla.org )


Important note about static link:

	"We don't support linking with NSPR's static libraries.
	Our binary distributions only contain DLLs.

	The reason we still generate the static libraries when
	we build NSPR from source code is that one of the NSS
	crypto libraries (nssckbi.dll) uses the NSPR static
	libraries.  That bug will be fixed in the upcoming
	NSS 3.11 release.

	If you really need to use NSPR static libraries, you
	can eliminate the unresolved externals you reported
	by compiling all of your files with the -D_NSPR_BUILD_
	compiler flag.  Again, you should only do this if you
	must use NSPR static libraries.  Please note the following
	two problems:
	1. Threads created outside NSPR can call NSPR functions
	with no problem, but their PRThread structures will be
	leaked when they terminate, because we free their
	PRThread structures in the DllMain function of
	libnspr4.dll/nspr4.dll.  This is not an issue if all
	the threads that call NSPR functions are created by
	NSPR (PR_CreateThread).
	2. All the NSPR public functions will be exported from
	your executable or DLL because they are defined with the
	__declspec(dllexport) keyword (the value of the PR_IMPLEMENT
	macro)."
---- >8 ----
	"After I wrote my last reply, I thought of another problem:
	multiple copies of NSPR in a process.

	Your .so/.dll will contain a copy of NSPR if you link with
	NSPR static libraries.

	If the executable program or some other .so/.dll also uses
	NSPR (the .so/.dll), the process will end up with two copies
	of NSPR.  In the past, this caused problems on at least Windows
	and AIX."	
	Wan-Teh
	(source: http://www.nabble.com/Static-linking-causing-unresolved-externals-t629691.html)


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
