include ../subdirs.mk

_INSTALL_DIR := $(shell pwd)/$(shell uname)

TARGET_FILES = $(_INSTALL_DIR)/libs/librsvg.a
CFLAGS += -fPIC

configure: ./src/Makefile

./src/Makefile:
	cd src && GLIB_CFLAGS=$(CFLAGS) ./configure --enable-shared=no --enable-static=yes --prefix=$(_INSTALL_DIR) GLIB_CFLAGS="-I../../glib/src/.libs/include " GLIB_LIBS=../../glib/src/.libs/libs  LIBRSVG_CFLAGS=" " LIBRSVG_LIBS=" "

all clean distclean:: configure
	$(MAKE) -C src $@

all:: configure
	$(MAKE) -C src install
