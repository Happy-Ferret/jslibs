include ../subdirs.mk

ifeq ($(BITS),64)
	CONFIGURE_OPTIONS=--enable-64bit
endif

TARGET_FILES=$(INT_DIR)dist/lib/libnspr4.so

$(INT_DIR)Makefile:
	mkdir -p $(INT_DIR)
	cd $(INT_DIR) && ../src/configure $(CONFIGURE_OPTIONS) --disable-debug --enable-strip --enable-optimize=-O3

all clean distclean:: $(INT_DIR)Makefile
	$(MAKE) -C $(INT_DIR) $@
