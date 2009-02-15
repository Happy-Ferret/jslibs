include ../subdirs.mk

TARGET_FILES=$(INT_DIR)libz.a

CFLAGS += -fPIC

./src/Makefile:
	cd ./src/ && ./configure

$(TARGET_FILES): ./src/Makefile
	$(MAKE) -C ./src/ 
	mkdir -p $(INT_DIR)
	cp ./src/libz.a ./src/zlib.h $(INT_DIR)
	$(MAKE) -C ./src/ clean distclean

all:: $(TARGET_FILES)

clean::
	-[ -d "$(INT_DIR)" ] && rm -r ./$(INT_DIR)
