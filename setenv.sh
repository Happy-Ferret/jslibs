#!/bin/bash

[ -z "$BUILD" ] && BUILD=opt

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD/$BUILD
PATH=$PATH:$PWD/$BUILD

export LD_LIBRARY_PATH
export PATH

echo jslibs $BUILD environment set.
