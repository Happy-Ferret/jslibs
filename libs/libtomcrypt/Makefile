include ../subdirs.mk

TARGET_FILES = $(INT_DIR)libtomcrypt.a

$(TARGET_FILES):
	mkdir -p $(INT_DIR)
	CFLAGS="-DLTM_DESC -I../../libtommath/$(INT_DIR)" $(MAKE) -C ./src/ default 
	cp ./src/libtomcrypt.a $(INT_DIR)
	patch --version
	-patch -N -p0 -i libtomcrypt.patch #avoid conflict between src/makefile's clean goal and src/.svn/ content
	$(MAKE) -C ./src/ clean
	-patch -N -R -p0 -i libtomcrypt.patch

all:: $(TARGET_FILES)

clean::
	rm $(TARGET_FILES)
