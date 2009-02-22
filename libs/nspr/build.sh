# https://developer.mozilla.org/en/SpiderMonkey_Build_Documentation
# prerequisites:
#   autoconf version 2.13
#   MozillaBuild 1.3 package (for windows only) http://ftp.mozilla.org/pub/mozilla.org/mozilla/libraries/win32/MozillaBuildSetup-1.3.exe
#

if [ -z $BUILD ]; then
	BUILD=OPT
fi

if [ $(uname -o) == "Msys" ]; then
	BUILD_DIR_NAME=Win32_$BUILD	
else
	BUILD_DIR_NAME=$(uname -s)_$BUILD
fi

mkdir $BUILD_DIR_NAME
cd $BUILD_DIR_NAME

if [ $BUILD == "OPT" ]; then
	CONFIGURE_OPTIONS="--disable-debug --enable-strip --enable-optimize"
else
	CONFIGURE_OPTIONS="--enable-debug --disable-optimize --enable-debug-rtl"
fi

../src/configure $CONFIGURE_OPTIONS --enable-win32-target=WIN95

diff -q ./jsversion.h ./src/jsversion.h || cp ./jsversion.h ./src/

make

# ls $BUILD_DIR_NAME/dist/lib

