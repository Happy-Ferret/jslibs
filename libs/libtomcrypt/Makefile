include ../subdirs.mk

TARGET_FILES = $(INT_DIR)libtomcrypt.a

$(TARGET_FILES):
	mkdir -p $(INT_DIR)
	CFLAGS="-fPIC -DLTM_DESC -I../../libtommath/src -DXMALLOC=jl_malloc -DXCALLOC=jl_calloc -DXREALLOC=jl_realloc -DXFREE=jl_free" $(MAKE) -C ./src/ default
	mv ./src/libtomcrypt.a $(INT_DIR)
	patch --version
	-patch -N -p0 -i libtomcrypt.patch #avoid conflict between src/makefile's clean goal and src/.svn/ content
	$(MAKE) -C ./src/ clean
	-patch -N -R -p0 -i libtomcrypt.patch

all:: $(TARGET_FILES)

clean::
	-[ -d "./$(INT_DIR)" ] && rm -r ./$(INT_DIR)
