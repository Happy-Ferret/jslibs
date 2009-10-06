include ../subdirs.mk

ifeq ($(BUILD),dbg)
	_CONFIGURE_OPTIONS = --enable-debug --disable-optimize
else
	_CONFIGURE_OPTIONS = --enable-optimize --enable-strip
endif

ifeq ($(BITS),64)
	_CFLAGS += -m64
else
	_CFLAGS += -m32
endif

TARGET_FILES=$(INT_DIR)dist/bin/*.{so,dll}

./src/configure:
	cd ./src/ && ( autoconf2.13 || autoconf-2.13 )

$(INT_DIR)Makefile: ./src/configure
	mkdir -p $(INT_DIR)
	cd $(INT_DIR) && CXXFLAGS="$(_CFLAGS)" ../src/configure $(_CONFIGURE_OPTIONS) --with-windows-version=501 --disable-static --disable-profile-guided-optimization --disable-vista-sdk-requirements --disable-tests

all:: $(INT_DIR)Makefile
	diff -q ./jsversion.h ./src/jsversion.h || cp ./jsversion.h ./src/
	-patch -s -N ./src/jsutil.h ./jsutil.h.patch
	-patch -s -N ./src/jsutil.cpp ./jsutil.cpp.patch

all::
	$(MAKE) -C $(INT_DIR) $@

clean::
	rm ./src/configure
	-[ -d "./$(INT_DIR)" ] && rm -r ./$(INT_DIR)
