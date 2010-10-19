include ../subdirs.mk

TARGET_FILES=$(INT_DIR).libs/libfreetype.a

$(INT_DIR)Makefile:
	mkdir -p $(INT_DIR) && cd $(INT_DIR) && LDFLAGS="-L../../zlib/$(INT_DIR)" INCLUDES="-I../../zlib/$(INT_DIR)" ../src/configure --disable-shared --enable-static

distclean clean::
	-rm -R ./$(INT_DIR)

all:: $(INT_DIR)Makefile
	$(MAKE) -C $(INT_DIR) $@
