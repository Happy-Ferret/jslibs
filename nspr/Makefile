include ../common/subdirs.mk

TARGET_FILES = ./src/dist/lib/libnspr4.so

./src/Makefile:
	cd src && ./configure --disable-debug --enable-strip --with-pthreads --enable-optimize=-O3

all clean distclean:: ./src/Makefile
	$(MAKE) -C src $@
