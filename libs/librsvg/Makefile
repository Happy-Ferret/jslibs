include ../subdirs.mk

TARGET_FILES = ./src/.libs/librsvg.a
CFLAGS += -fPIC

_INSTALL_DIR=`pwd`/`uname`

configure: ./src/Makefile

./src/Makefile:
	cd src && ./configure --enable-shared=no --enable-static=yes --prefix=$(_INSTALL_DIR)

all clean distclean:: configure
	$(MAKE) -C src $@

all:: configure
	$(MAKE) -C src install
