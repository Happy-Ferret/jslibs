include ../subdirs.mk

#BUILD := $(shell echo $(BUILD) | tr A-Z a-z)

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
	
	
	# diff src/js/public/Utility.h.orig src/js/public/Utility.h -U 3 --label "" > Utility.h.patch
	-patch -r tmp.rej -N src/js/public/Utility.h Utility.h.patch

	# diff src/js/src/jsutil.cpp.orig src/js/src/jsutil.cpp -U 3 --label "" > jsutil.cpp.patch
	-patch -r tmp.rej -N src/js/src/jsutil.cpp jsutil.cpp.patch

	# diff src/js/src/jspubtd.h.orig src/js/src/jspubtd.h -U 3 --label "" > jspubtd.h.patch
	-patch -r tmp.rej -N src/js/src/jspubtd.h jspubtd.h.patch
	
	# rm tmp.rej

	mkdir -p $(INT_DIR)
	cd $(INT_DIR) && CXXFLAGS="$(_CFLAGS)" ../src/js/src/configure $(_CONFIGURE_OPTIONS) --with-windows-version=601 --disable-static --disable-profile-guided-optimization --disable-vista-sdk-requirements --disable-tests --disable-intl-api
	# --enable-intl-api (require to remove --disable-intl-api and to add jslibs/libs/js/src/intl directory)
	# --enable-ctypes (require to be linked with NSPR)

all:: $(INT_DIR)Makefile
	$(MAKE) -C $(INT_DIR) $@

clean::
	rm ./src/js/src/configure
	-[ -d "./$(INT_DIR)" ] && rm -r ./$(INT_DIR)
