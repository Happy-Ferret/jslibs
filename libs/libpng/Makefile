include ../subdirs.mk

TARGET_FILES = ./src/.libs/libpng.a

./src/Makefile:
	cd ./src && LDFLAGS="-L../../zlib/$(BUILD)" INCLUDES="-I../../zlib/src" ./configure --disable-shared --enable-static

all clean distclean:: ./src/Makefile
	LDFLAGS="-L../../zlib/$(BUILD)" INCLUDES="-I../../zlib/src" $(MAKE) -C src $@
