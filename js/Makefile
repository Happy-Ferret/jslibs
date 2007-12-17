include ../common/subdirs.mk

ifeq ($(BUILD),dbg)
	LIBJS_DIR = Linux_All_DBG.OBJ
	LIBJS_MAKE_OPTIONS =
	CFLAGS =
else
	LIBJS_DIR = Linux_All_OPT.OBJ
	LIBJS_MAKE_OPTIONS = BUILD_OPT=1
	CFLAGS = -O3
endif

TARGET_FILES = ./src/$(LIBJS_DIR)/libjs.so

all clean::
	$(MAKE) -C src -f Makefile.ref $@ $(LIBJS_MAKE_OPTIONS)
