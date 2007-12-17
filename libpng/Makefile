include ../common/subdirs.mk

TARGET_FILES = ./src/.libs/libpng.a

./src/Makefile:
	LDFLAGS="-L../../zlib/src" INCLUDES="-I../../zlib/src" cd src && ./configure --disable-shared

all clean distclean:: ./src/Makefile
	LDFLAGS="-L../../zlib/src" INCLUDES="-I../../zlib/src" $(MAKE) -C src $@
