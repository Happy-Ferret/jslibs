include ../subdirs.mk

TARGET_FILES = ./src/.libs/libxml2.a
CFLAGS += -fPIC

_INSTALL_DIR=`pwd`/`uname`

configure: ./src/Makefile

./src/Makefile:
	cd src && ./configure --enable-shared=no --enable-static=yes --with-debug=no --with-docbook=no --with-threads=no --with-ftp=no --with-http=no --with-zlib=../../zlib/src --prefix=$(_INSTALL_DIR)

all clean distclean:: configure
	$(MAKE) -C src $@

all:: configure
	$(MAKE) -C src install
