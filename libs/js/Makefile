include ../subdirs.mk

_DEST_DIR=./$(BUILD)/

ifeq ($(BUILD),dbg)
	_MAKE_OPTIONS=BUILD_IDG=1
	_INTERMEDIATE_DIR=./src/`uname`_All_DBG.OBJD/
	POSBUILD=
else
	_MAKE_OPTIONS=BUILD_OPT=1
	_INTERMEDIATE_DIR=./src/`uname`_All_OPT.OBJ/
	POSBUILD=strip $(_DEST_DIR)*.so
endif

ifeq ($(BITS),32)
	_CFLAGS += -m32
endif

ifeq ($(BITS),64)
	_CFLAGS += -m64
endif

TARGET_FILES=./src/$(BUILD)/libjs.so

all::
	diff -q ./jsversion.h ./src/jsversion.h || cp ./jsversion.h ./src/

all clean::
	$(MAKE) -C src -f Makefile.ref $@ $(_MAKE_OPTIONS) XCFLAGS="$(_CFLAGS)"

all::
	mkdir -p $(_DEST_DIR)
	cp $(_INTERMEDIATE_DIR)*.h $(_DEST_DIR)
	cp $(_INTERMEDIATE_DIR)*.so $(_DEST_DIR)
	$(POSBUILD)
