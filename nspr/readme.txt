download:
	ftp://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.6.1/WINNT5.0_OPT.OBJ

directory structure
	.
	..
	include/nspr
	lib
	readme.txt

misc:
	static link: define _NSPR_BUILD_, link  with  ...\nspr-4.6\lib\libnspr4_s.lib
	API reference: http://developer.mozilla.org/en/docs/NSPR_API_Reference
	Build procedure: http://www.mozilla.org/projects/nspr/eng-process/build.html

try last version:
	ftp://ftp.mozilla.org/pub/mozilla.org/nspr/releases/


Linux compilation:
	./configure --disable-debug --enable-strip --enable-nspr-threads --enable-optimize=-O3
	make

<...>
Hi,

I added two wiki pages on developer.mozilla.org for NSPR build
and test instructions:

http://developer.mozilla.org/en/docs/NSPR_build_instructions

http://developer.mozilla.org/en/docs/Running_NSPR_tests

I welcome your review comments.

Wan-Teh 

<...>