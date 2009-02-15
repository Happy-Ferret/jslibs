# https://developer.mozilla.org/en/SpiderMonkey_Build_Documentation
# prerequisites:
#   autoconf version 2.13
#   MozillaBuild 1.3 package (for windows only) http://ftp.mozilla.org/pub/mozilla.org/mozilla/libraries/win32/MozillaBuildSetup-1.3.exe
#

if [ -z $BUILD ]; then
	BUILD=OPT
fi

cd src
autoconf-2.13 || autoconf2.13
cd ..

BUILD_DIR_NAME=$(uname -s)_$BUILD

mkdir $BUILD_DIR_NAME
cd $BUILD_DIR_NAME

if [ $BUILD == OPT ]; then
	CONFIGURE_OPTIONS="--enable-strip"
else
	CONFIGURE_OPTIONS="--enable-debug --disable-optimize"
	# --enable-md 
	# export CXXFLAGS="-g3"
fi

../src/configure $CONFIGURE_OPTIONS --with-windows-version=501 --disable-static --disable-profile-guided-optimization --disable-vista-sdk-requirements

diff -q ./jsversion.h ./src/jsversion.h || cp ./jsversion.h ./src/

make

# ls $BUILD_DIR_NAME/dist/lib

