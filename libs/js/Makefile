include ../subdirs.mk

ifeq ($(BUILD),dbg)
	_CONFIGURE_OPTIONS = --enable-debug --disable-optimize --enable-gczeal
else
	_CONFIGURE_OPTIONS = --enable-optimize --enable-strip
endif

ifeq ($(BITS),64)
	_CFLAGS += -m64
else
	_CFLAGS += -m32
endif


TARGET_FILES=$(INT_DIR)dist/bin/*.{so,dll}

./src/js/src/configure:
	cd ./src/js/src/ && ( autoconf2.13 || autoconf-2.13 )

$(INT_DIR)Makefile: ./src/js/src/configure
	diff -q ./jsversion.h ./src/js/src/jsversion.h || cp ./jsversion.h ./src/js/src/
	# -patch -N ./src/js/src/jsutil.h jsutil.h.patch
	# -patch -N ./src/js/src/jsutil.cpp jsutil.cpp.patch
	mkdir -p $(INT_DIR)
	cd $(INT_DIR) && CXXFLAGS="$(_CFLAGS)" ../src/js/src/configure $(_CONFIGURE_OPTIONS) --with-windows-version=601 --disable-static --disable-profile-guided-optimization --disable-vista-sdk-requirements --disable-tests
	 # --enable-ctypes (require to be linked with NSPR)

all:: $(INT_DIR)Makefile
	$(MAKE) -C $(INT_DIR) $@

clean::
	rm ./src/js/src/configure
	-[ -d "./$(INT_DIR)" ] && rm -r ./$(INT_DIR)
