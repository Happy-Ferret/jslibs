include ../subdirs.mk

TARGET_FILES=$(INT_DIR)libz.a

ifeq ($(BITS),64)
	CFLAGS+=-fPIC
endif

$(TARGET_FILES):
	cd ./src/ && ./configure --libdir=/usr/lib64
	$(MAKE) -C ./src/ clean all CFLAGS=$(CFLAGS)
	mkdir -p $(INT_DIR)
	cp ./src/libz.a ./src/zlib.h ./src/zconf.h $(INT_DIR)
	$(MAKE) -C ./src/ clean distclean

all:: $(TARGET_FILES)

clean::
	-[ -d "$(INT_DIR)" ] && rm -r ./$(INT_DIR)
