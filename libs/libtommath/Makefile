include ../subdirs.mk

TARGET_FILES = $(INT_DIR)libtommath.a

$(TARGET_FILES):
	mkdir -p $(INT_DIR)
	$(MAKE) -C ./src/ default
	cp ./src/libtommath.a $(INT_DIR)
	$(MAKE) -C ./src/ clean

all:: $(TARGET_FILES)
	
clean::
	rm $(TARGET_FILES)
