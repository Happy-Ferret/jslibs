TARGET     = jsimage.so
SRC        = $(wildcard *.cpp)
INCLUDES   = -I../../libs/zlib/src -I../../libs/libpng/src -I../../libs/libjpeg/$(INT_DIR) -I../../libs/libjpeg/src
STATICLIBS = ../../libs/libpng/$(INT_DIR).libs/libpng.a ../../libs/libjpeg/$(INT_DIR).libs/libjpeg.a ../../libs/zlib/$(INT_DIR)libz.a
SHAREDLIBS = -ldl -lpthread
DEPENDS    = ../../libs/libpng/Makefile ../../libs/libjpeg/Makefile

include ../rules.mk

