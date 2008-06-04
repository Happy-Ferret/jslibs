include ../common/subdirs.mk

TARGET_FILES = ./src/.libs/libjpeg.a

./src/Makefile:
	cd src && ./configure --enable-static --disable-shared

all clean distclean:: ./src/Makefile
	$(MAKE) -C src $@
