include ../subdirs.mk

TARGET_FILES = $(INT_DIR).libs/libjpeg.a

CFLAGS += -Dmalloc=jl_malloc_fct -Dcalloc=jl_calloc_fct -Drealloc=jl_realloc_fct -Dfree=jl_free_fct

$(INT_DIR)Makefile:
	mkdir -p $(INT_DIR)
	cd $(INT_DIR) && CFLAGS="$(CFLAGS)" ../src/configure --enable-static --disable-shared
	cp ./dummy.c src/wrjpgcom.c
	cp ./dummy.c src/djpeg.c
	cp ./dummy.c src/cjpeg.c
	cp ./dummy.c src/jpegtran.c

distclean clean::
	-rm -R ./$(INT_DIR)

all:: $(INT_DIR)Makefile
	$(MAKE) -C $(INT_DIR) $@

