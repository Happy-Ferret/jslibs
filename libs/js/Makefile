include ../subdirs.mk

ifeq ($(BUILD),dbg)
	_MAKE_OPTIONS=BUILD_IDG=1
	_INTERMEDIATE_DIR=./src/`uname`_All_DBG.OBJD/
	POSBUILD=
else
	_MAKE_OPTIONS=BUILD_OPT=1
	_INTERMEDIATE_DIR=./src/`uname`_All_OPT.OBJ/
	POSBUILD=strip $(DEST_DIR)*.so
endif

ifeq ($(BITS),32)
	_CFLAGS += -m32
endif

ifeq ($(BITS),64)
	_CFLAGS += -m64
endif

TARGET_FILES=$(DEST_DIR)libjs.so

all::
	diff -q ./jsversion.h ./src/jsversion.h || cp ./jsversion.h ./src/
	echo // > ./src/js-config.h

all clean::
	$(MAKE) -C src -f Makefile.ref $@ $(_MAKE_OPTIONS) XCFLAGS="$(_CFLAGS)"

all::
	mkdir -p $(DEST_DIR)
	cp $(_INTERMEDIATE_DIR)*.h $(DEST_DIR)
	cp $(_INTERMEDIATE_DIR)*.so $(DEST_DIR)
	$(POSBUILD)

copy: ;
