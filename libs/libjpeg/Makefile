include ../subdirs.mk

TARGET_FILES = $(INT_DIR).libs/libjpeg.a

$(INT_DIR)Makefile:
	mkdir -p $(INT_DIR)
	cd $(INT_DIR) && ../src/configure --enable-static --disable-shared

all clean distclean:: $(INT_DIR)Makefile
	$(MAKE) -C $(INT_DIR) $@

# -Dmalloc=jl_malloc_wrapper -Dcalloc=jl_calloc_wrapper -Drealloc=jl_realloc_wrapper -Dfree=jl_free_wrapper
