include ../subdirs.mk

TARGET_FILES = $(INT_DIR).libs/libpng.a

$(INT_DIR)Makefile:
	mkdir -p $(INT_DIR)
	cd $(INT_DIR) && LDFLAGS="-L../../zlib/$(INT_DIR)" INCLUDES="-I../../zlib/$(INT_DIR)" ../src/configure --disable-shared --enable-static

all clean distclean:: $(INT_DIR)Makefile
	#INCLUDES="-I../../zlib/src" $(MAKE) -C src $@
	$(MAKE) -C $(INT_DIR) $@
