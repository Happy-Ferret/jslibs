include ../common/subdirs.mk

TARGET_FILES = ./src/libz.a
CFLAGS += -fPIC

./src/Makefile:
	cd src && ./configure

all clean distclean:: ./src/Makefile
	$(MAKE) -C src $@
