include ../../common/subdirs.mk

ifeq ($(BUILD),dbg)
	LIBJS_DIR = Linux_All_DBG.OBJ
	LIBJS_MAKE_OPTIONS =
	CFLAGS = -DJS_GCMETER -DJS_HASHMETER
# -DGC_MARK_DEBUG -DJS_ARENAMETER
else
	LIBJS_DIR = Linux_All_OPT.OBJ
	LIBJS_MAKE_OPTIONS = BUILD_OPT=1
	CFLAGS = -O3
	POSBUILD = strip $(TARGET_FILES)
endif

TARGET_FILES = ./src/$(LIBJS_DIR)/libjs.so

all::
	diff -q ./jsconfig.h ./src/jsconfig.h || cp ./jsconfig.h ./src/

all clean::
	$(MAKE) -C src -f Makefile.ref $@ $(LIBJS_MAKE_OPTIONS) XCFLAGS="$(CFLAGS)"
	$(POSBUILD)
