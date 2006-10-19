download:
	ftp://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.6.1/WINNT5.0_OPT.OBJ

directory structure
	.
	..
	include
	lib
	readme.txt

misc:
	static link: define _NSPR_BUILD_, link  with  ...\nspr-4.6\lib\libnspr4_s.lib
