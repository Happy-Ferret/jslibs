include ../common/subdirs.mk

TARGET_FILES = ./src/libz.a

./src/Makefile:
	cd src && ./configure

all clean distclean:: ./src/Makefile
	$(MAKE) -C src $@
