include ../common/subdirs.mk

TARGET_FILES = ./src/dist/lib/libnspr4.so

ifeq ($(BITS),64)
	CONFIGUREFLAGS += --enable-64bit
endif

./src/Makefile:
	cd src && ./configure --disable-debug --enable-strip --enable-optimize=-O3 $(CONFIGUREFLAGS)

all clean distclean:: ./src/Makefile
	$(MAKE) -C src $@
