Windows build:
-------------
which libraries should be ignored depending on which run-time library you want to use. (source: http://msdn2.microsoft.com/en-us/library/ms931581.aspx)

	use this run-time library                   | Ignore these libraries
	Single-threaded (libc.lib)                  | libcmt.lib, msvcrt.lib, libcd.lib, libcmtd.lib, msvcrtd.lib
	Multithreaded (libcmt.lib)                  | libc.lib, msvcrt.lib, libcd.lib, libcmtd.lib, msvcrtd.lib
	Multithreaded using DLL (msvcrt.lib)        | libc.lib, libcmt.lib, libcd.lib, libcmtd.lib, msvcrtd.lib
	Debug Single-threaded (libcd.lib)           | libc.lib, libcmt.lib, msvcrt.lib, libcmtd.lib, msvcrtd.lib
	Debug Multithreaded (libcmtd.lib)           | libc.lib, libcmt.lib, msvcrt.lib, libcd.lib, msvcrtd.lib
	Debug Multithreaded using DLL (msvcrtd.lib) | libc.lib, libcmt.lib, msvcrt.lib, libcd.lib, libcmtd.lib

cf.  jslibs_debug.vsprops, jslibs_release.vsprops

eg. /NODEFAULTLIB:libc.lib, libcmt.lib, msvcrt.lib, libcd.lib, libcmtd.lib
