[ -z "$BUILD" ] && BUILD=opt

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD/$BUILD
export PATH=$PATH:$PWD/$BUILD

echo jslibs $BUILD environment set.
