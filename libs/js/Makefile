include ../subdirs.mk

ifeq ($(BUILD),dbg)
	LIBJS_MAKE_OPTIONS=BUILD_IDG=1
	LIBJS_DIR=`uname`_All_DBG.OBJD
#	CFLAGS=-DJS_GCMETER -DJS_HASHMETER
# -DGC_MARK_DEBUG -DJS_ARENAMETER
	POSBUILD=
else
	LIBJS_MAKE_OPTIONS=BUILD_OPT=1
	LIBJS_DIR=`uname`_All_OPT.OBJ
#	CFLAGS = -O3
	POSBUILD=strip $(TARGET_FILES)
endif

ifeq ($(BITS),32)
	CFLAGS += -m32
endif

ifeq ($(BITS),64)
	CFLAGS += -m64
endif

TARGET_FILES=./src/$(LIBJS_DIR)/libjs.so

all::
	diff -q ./jsversion.h ./src/jsversion.h || cp ./jsversion.h ./src/

all clean::
	$(MAKE) -C src -f Makefile.ref $@ $(LIBJS_MAKE_OPTIONS) XCFLAGS="$(CFLAGS)"
	$(POSBUILD)
