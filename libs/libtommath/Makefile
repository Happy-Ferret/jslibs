include ../subdirs.mk

TARGET_FILES = $(INT_DIR)libtommath.a

$(TARGET_FILES):
	mkdir -p $(INT_DIR)
	CFLAGS="-fPIC -DXMALLOC=jl_malloc_fct -DXCALLOC=jl_calloc_fct -DXREALLOC=jl_realloc_fct -DXFREE=jl_free_fct" $(MAKE) -C ./src/ default
	mv ./src/libtommath.a $(INT_DIR)
	$(MAKE) -C ./src/ clean

all:: $(TARGET_FILES)
	
clean::
	-[ -d "./$(INT_DIR)" ] && rm -r ./$(INT_DIR)
