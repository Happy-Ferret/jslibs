include ../subdirs.mk

TARGET_FILES = src/objs/.libs/libfreetype.a

./src/Makefile:
	LDFLAGS="-L../../zlib/$(BUILD)" INCLUDES="-I../../zlib/src" cd src && ./configure --disable-shared --enable-static

all clean distclean:: ./src/Makefile
	LDFLAGS="-L../../zlib/src" INCLUDES="-I../../zlib/src" $(MAKE) -C src $@
