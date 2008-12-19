include ../subdirs.mk

TARGET_FILES = ./src/libfcgi/.libs/libfcgi.a
CFLAGS += -fPIC

./src/Makefile:
	cd src && sh ./configure

all clean distclean:: ./src/Makefile
	$(MAKE) -C src $@
