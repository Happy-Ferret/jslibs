include ../subdirs.mk

CURRENTDIR=$(shell pwd)

TARGET_FILES = $(INT_DIR)lib/.libs/libvorbis.a

$(INT_DIR)Makefile:
	mkdir -p $(INT_DIR)
	#LDFLAGS="-L../../libogg/$(INT_DIR)src/.libs/" INCLUDES="-I../../libogg/$(INT_DIR)/include/ -I../../libogg/src/include/"
	cd $(INT_DIR) && ../src/configure --with-ogg-libraries="$(CURRENTDIR)/../libogg/$(INT_DIR)src/.libs/" --with-ogg-includes="$(CURRENTDIR)/../libogg/$(INT_DIR)include/" --enable-shared=no --enable-static=yes --disable-dependency-tracking --disable-oggtest

all:: $(INT_DIR)Makefile
	$(MAKE) -C $(INT_DIR) all

clean distclean:: $(INT_DIR)Makefile
	$(MAKE) -C $(INT_DIR) $@
