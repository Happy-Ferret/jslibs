include ../subdirs.mk

TARGET_FILES = $(INT_DIR)src/.libs/libsndfile.a

$(INT_DIR)Makefile:
	mkdir -p $(INT_DIR)
	cd $(INT_DIR) && ../src/configure --enable-shared=no --enable-static=yes --disable-dependency-tracking

all clean distclean:: $(INT_DIR)Makefile
	$(MAKE) -C $(INT_DIR) $@
