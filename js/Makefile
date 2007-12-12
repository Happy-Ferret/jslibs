ifeq ($(BUILD),dbg)
	BUILD = dbg
	LIBJS_DIR = Linux_All_DBG.OBJ
	LIBJS_MAKE_OPTIONS =
else
	BUILD = opt
	LIBJS_DIR = Linux_All_OPT.OBJ
	LIBJS_MAKE_OPTIONS = BUILD_OPT=1
endif


.PHONY: all clean install
all clean install:
	cd src && $(MAKE) -f Makefile.ref $(MAKECMDGOALS) $(LIBJS_MAKE_OPTIONS)
