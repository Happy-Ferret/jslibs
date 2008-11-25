TARGET     = jsimage.so
SRC        = $(wildcard *.cpp)
INCLUDES   = -I../../libs/zlib/src -I../../libs/libpng/src -I../../libs/libjpeg/src
STATICLIBS = ../../libs/libpng/$(BUILD)/libpng.a ../../libs/libjpeg/$(BUILD)/libjpeg.a ../../libs/zlib/$(BUILD)/libz.a
SHAREDLIBS = -ldl -lpthread
DEPENDS    = ../../libs/libpng/Makefile ../../libs/libjpeg/Makefile

include ../rules.mk

