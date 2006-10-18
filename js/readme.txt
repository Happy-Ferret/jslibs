cvs:
	cvs -d :pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot login
	cvs -d :pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot co -d src mozilla/js/src

modify:
	src/jsconfig.h: enable JS_HAS_XDR_FREEZE_THAW in the last JS_VERSION

directory structure:
	.
	..
	src
	readme.txt


help:
	http://developer.mozilla.org/en/docs/Introduction_to_the_JavaScript_shell