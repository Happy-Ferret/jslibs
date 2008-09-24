include ../subdirs.mk

_INSTALL_DIR=`pwd`/`uname`

TARGET_FILES = $(_INSTALL_DIR)libxml2.a
CFLAGS += -fPIC

configure: ./src/Makefile

./src/Makefile:
	cd src && ./configure --enable-shared=no --enable-static=yes --without-debug --without-docbook --without-threads --without-ftp --without-http --with-zlib=../../zlib/src --prefix=$(_INSTALL_DIR)

all clean distclean:: configure
	$(MAKE) -C src $@

all:: configure
	$(MAKE) -C src install
