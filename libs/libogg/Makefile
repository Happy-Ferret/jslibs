include ../subdirs.mk

THISDIR=$(shell pwd)

TARGET_FILES = $(INT_DIR)src/.libs/libogg.a

$(INT_DIR)Makefile:
	mkdir -p $(INT_DIR)
	cd $(INT_DIR) && ../src/configure --enable-shared=no --enable-static=yes --disable-dependency-tracking
	# --prefix=$(THISDIR)/$(INT_DIR)

all:: $(INT_DIR)Makefile
	$(MAKE) -C $(INT_DIR)src/ all
	cp src/include/ogg/*.h $(INT_DIR)include/ogg/

clean distclean:: $(INT_DIR)Makefile
	$(MAKE) -C $(INT_DIR) $@
