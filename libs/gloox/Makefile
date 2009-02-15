include ../subdirs.mk

ifeq ($(BITS),64)
	_CFLAGS += -m64
else
	_CFLAGS += -m32
endif

TARGET_FILES=$(INT_DIR)

$(INT_DIR)Makefile: ./src/configure
	mkdir -p $(INT_DIR) && cd $(INT_DIR) && CXXFLAGS=$(_CFLAGS) ../src/configure $(_CONFIGURE_OPTIONS) --enable-shared=no --disable-dependency-tracking --with-zlib=../../zlib/$(INT_DIR)

all clean:: $(INT_DIR)Makefile
	$(MAKE) -C $(INT_DIR) $@
