include ../subdirs.mk

TARGET_FILES = $(INT_DIR).libs/libjpeg.a

$(INT_DIR)Makefile:
	mkdir -p $(INT_DIR)
	cd $(INT_DIR) && ../src/configure --enable-static --disable-shared

all clean distclean:: $(INT_DIR)Makefile
	$(MAKE) -C $(INT_DIR) $@
