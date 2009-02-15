include ../subdirs.mk

TARGET_FILES = $(INT_DIR)libfcgi/.libs/libfcgi.a
CFLAGS += -fPIC

$(INT_DIR)Makefile:
	mkdir -p $(INT_DIR)
	cd $(INT_DIR) && sh ../src/configure --enable-shared=no --disable-dependency-tracking

all clean distclean:: $(INT_DIR)Makefile
	$(MAKE) -C $(INT_DIR) $@
