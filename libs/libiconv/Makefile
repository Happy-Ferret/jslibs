include ../subdirs.mk

TARGET_FILES=$(INT_DIR)lib/.libs/libiconv.a

$(INT_DIR)Makefile:
	mkdir -p $(INT_DIR)
	cd $(INT_DIR) && am_cv_func_iconv="no" ../src/configure --disable-rpath --enable-static=yes --enable-shared=no --disable-dependency-tracking am_cv_proto_iconv_arg1="const"

all:: $(INT_DIR)Makefile
	$(MAKE) -C $(INT_DIR) $@

clean distclean::
	-[ -d "./$(INT_DIR)" ] && rm -r ./$(INT_DIR)
