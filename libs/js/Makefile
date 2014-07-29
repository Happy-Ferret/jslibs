include ../subdirs.mk

_CONFIGURE_OPTIONS := --with-windows-version=601 --enable-nspr-build --without-intl-api --enable-ctypes --disable-tests --disable-static --disable-gcgenerational
#  --enable-threadsafe --disable-gcgenerational --enable-exact-rooting (--enable-intl-api require jslibs/libs/js/src/intl directory)

ifeq ($(BUILD),dbg)
	_CONFIGURE_OPTIONS += --enable-debug --disable-optimize --enable-gczeal
else
	_CONFIGURE_OPTIONS += --enable-strip --enable-optimize
# --enable-optimize=-Ox 
# _CFLAGS += -GL
# _LDFLAGS += -LTCG
endif

ifeq ($(BITS),64)
	_CFLAGS += -m64
else
	_CFLAGS += -m32
endif


TIP_ID := mozilla-central-????????????

TIP_NAME := $(shell ls -td $(TIP_ID) | head -1)

TARGET_FILES := $(INT_DIR)dist/bin/*.{so,dll}


.INTERMEDIATE: tip.zip
.PRECIOUS: tip.zip

tip.zip:
	wget http://hg.mozilla.org/mozilla-central/archive/tip.zip
	touch -c --reference=$< $@
	
src.zip: tip.zip
	7z x -aos $< $(TIP_ID)/{toolkit/*,nsprpub/*,mfbt/*,python/*,testing/*,build/*,config/*,intl/*,js/public/*,js/src/*,dom/bindings/mozwebidlcodegen/*,media/webrtc/trunk/tools/gyp/*,layout/tools/reftest/*,modules/zlib/*,moz.build,Makefile.in,mozilla-config.h.in,aclocal.m4,configure.in} -x\!$(TIP_ID)/{js/src/jit-test/*,js/src/tests/*,js/src/v8/*} > /dev/null
	echo -n $(TIP_NAME) > ./$(TIP_NAME)/tip.txt
	mv $(TIP_NAME) src
	touch -c --reference=$< src
	7z -mx9 a $@ src > /dev/null
	touch -c --reference=$< $@
	touch -c --reference=$< src

src: src.zip
	7z x $< > /dev/null
	touch -c --reference=$< $@

./src/js/src/configure: src
	diff -q ./jsversion.h ./src/js/src/jsversion.h || cp ./jsversion.h ./src/js/src/
	
	#	diff src/js/public/Utility.h.orig src/js/public/Utility.h -U 3 --label "" > Utility.h.patch
	-patch -r tmp.rej -N src/js/public/Utility.h Utility.h.patch

	#	diff src/js/src/jsutil.cpp.orig src/js/src/jsutil.cpp -U 3 --label "" > jsutil.cpp.patch
	-patch -r tmp.rej -N src/js/src/jsutil.cpp jsutil.cpp.patch

	#	diff src/js/src/jspubtd.h.orig src/js/src/jspubtd.h -U 3 --label "" > jspubtd.h.patch
	-patch -r tmp.rej -N src/js/src/jspubtd.h jspubtd.h.patch

	cd $</js/src/ && ( autoconf2.13 || autoconf-2.13 )
	./src/js/src/configure --help > configure-help.txt

$(INT_DIR)Makefile: ./src/js/src/configure
	mkdir -p $(INT_DIR)
	cd $(INT_DIR) && CXXFLAGS="$(_CFLAGS)" LDFLAGS="$(_LDFLAGS)" ../src/js/src/configure $(_CONFIGURE_OPTIONS)

all:: $(INT_DIR)Makefile
	$(MAKE) -C $(INT_DIR) $@

clean::
	rm ./src/js/src/configure
	-[ -d "./$(INT_DIR)" ] && rm -r ./$(INT_DIR)
