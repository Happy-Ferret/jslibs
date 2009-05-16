TARGET     = jsiconv.so
SRC        = $(wildcard *.cpp)
INCLUDES   = -I../../libs/libiconv/$(INT_DIR)include
STATICLIBS = -L../../libs/libiconv/$(INT_DIR)lib/.libs -liconv
SHAREDLIBS =
DEPENDS    = ../../libs/libiconv/Makefile

include ../rules.mk
