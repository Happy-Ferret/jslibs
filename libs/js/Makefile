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


TARGET_FILES=$(INT_DIR)dist/bin/*.{so,dll}

TIP_NAME=mozilla-central-????????????

tip.zip:
	wget http://hg.mozilla.org/mozilla-central/archive/tip.zip

./src.zip: tip.zip
	7z x tip.zip $(TIP_NAME)/{nsprpub/*,mfbt/*,python/*,testing/*,build/*,config/*,intl/*,js/public/*,js/src/*,dom/bindings/mozwebidlcodegen/*,media/webrtc/trunk/tools/gyp/*,moz.build,Makefile.in,mozilla-config.h.in,aclocal.m4,configure.in} -x\!$(TIP_NAME)/{js/src/jit-test/*,js/src/tests/*,js/src/v8/*} > /dev/null
	echo -n $(TIP_NAME) > tip.txt
	mv $(TIP_NAME) src
	mv tip.txt src
	touch -c src
	7z -mx9 a src.zip src > /dev/null

./src: src.zip
	7z x src.zip > /dev/null
	touch -c ./src

./src/js/src/configure: ./src
	cd ./src/js/src/ && ( autoconf2.13 || autoconf-2.13 )

$(INT_DIR)Makefile: ./src/js/src/configure
	diff -q ./jsversion.h ./src/js/src/jsversion.h || cp ./jsversion.h ./src/js/src/
	
	#	diff src/js/public/Utility.h.orig src/js/public/Utility.h -U 3 --label "" > Utility.h.patch
	-patch -r tmp.rej -N src/js/public/Utility.h Utility.h.patch

	#	diff src/js/src/jsutil.cpp.orig src/js/src/jsutil.cpp -U 3 --label "" > jsutil.cpp.patch
	-patch -r tmp.rej -N src/js/src/jsutil.cpp jsutil.cpp.patch

	#	diff src/js/src/jspubtd.h.orig src/js/src/jspubtd.h -U 3 --label "" > jspubtd.h.patch
	-patch -r tmp.rej -N src/js/src/jspubtd.h jspubtd.h.patch
	
	# -patch -r xtmp.rej -N src/js/src win32thread.patch
	
	# rm tmp.rej

	mkdir -p $(INT_DIR)
	cd $(INT_DIR) && CXXFLAGS="$(_CFLAGS)" ../src/js/src/configure $(_CONFIGURE_OPTIONS) --enable-threadsafe --enable-gcgenerational --enable-exact-rooting --without-intl-api --enable-ctypes  --with-windows-version=601 --disable-static --disable-profile-guided-optimization --disable-vista-sdk-requirements --disable-tests
	#	--enable-intl-api (require to remove --disable-intl-api and to add jslibs/libs/js/src/intl directory)

all:: $(INT_DIR)Makefile
	$(MAKE) -C $(INT_DIR) $@

clean::
	rm ./src/js/src/configure
	-[ -d "./$(INT_DIR)" ] && rm -r ./$(INT_DIR)
