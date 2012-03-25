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

# -I.. : engine
# -I../.. : shell
#_CFLAGS += -DJS_USE_CUSTOM_ALLOCATOR=1 -I.. -I../..

TARGET_FILES=$(INT_DIR)dist/bin/*.{so,dll}

./src/js/src/configure:
	cd ./src/js/src/ && ( autoconf2.13 || autoconf-2.13 )

$(INT_DIR)Makefile: ./src/js/src/configure
	diff -q ./jsversion.h ./src/js/src/jsversion.h || cp ./jsversion.h ./src/js/src/
	# diff src/js/src/jsfriendapi.h.orig src/js/src/jsfriendapi.h -U 3 --label "" > jsfriendapi.h.patch
	-patch -N src/js/src/jsfriendapi.h jsfriendapi.h.patch
	# diff src/js/public/Utility.h.orig src/js/public/Utility.h -U 3 --label "" > Utility.h.patch
	-patch -N src/js/public/Utility.h Utility.h.patch
	# diff src/js/src/jsutil.cpp.orig src/js/src/jsutil.cpp -U 3 --label "" > jsutil.cpp.patch
	-patch -N src/js/src/jsutil.cpp jsutil.cpp.patch
	mkdir -p $(INT_DIR)
	cd $(INT_DIR) && CXXFLAGS="$(_CFLAGS)" ../src/js/src/configure $(_CONFIGURE_OPTIONS) --with-windows-version=601 --disable-static --disable-profile-guided-optimization --disable-vista-sdk-requirements --disable-tests
	# --enable-ctypes (require to be linked with NSPR)

all:: $(INT_DIR)Makefile
	$(MAKE) -C $(INT_DIR) $@

clean::
	rm ./src/js/src/configure
	-[ -d "./$(INT_DIR)" ] && rm -r ./$(INT_DIR)
