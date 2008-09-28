include ../subdirs.mk

_INSTALL_DIR := $(shell pwd)/$(shell uname)

TARGET_FILES = $(_INSTALL_DIR)/libs/glib.a
CFLAGS += -fPIC

configure: ./src/Makefile

./src/Makefile:
	cd src && ./configure --enable-shared=no --enable-static=yes --enable-debug=no --without-threads --prefix=$(_INSTALL_DIR)

all clean distclean:: configure
	$(MAKE) -C src $@

all:: configure
	$(MAKE) -C src install
