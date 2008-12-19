TARGET     = jsfastcgi.so
SRC        = $(wildcard *.cpp)
INCLUDES   = -I../../libs/fastcgi/src/include
STATICLIBS = -L../../libs/fastcgi/src/libfcgi/.libs/ -lfcgi
SHAREDLIBS =
DEPENDS    = ../../libs/fastcgi/Makefile

include ../rules.mk
