include ../subdirs.mk

TARGET_FILES = $(INT_DIR)libtommath.a

$(TARGET_FILES):
	mkdir -p $(INT_DIR)
	CFLAGS="-fPIC" $(MAKE) -C ./src/ default
	mv ./src/libtommath.a $(INT_DIR)
	$(MAKE) -C ./src/ clean

all:: $(TARGET_FILES)
	
clean::
	-[ -d "./$(INT_DIR)" ] && rm -r ./$(INT_DIR)
