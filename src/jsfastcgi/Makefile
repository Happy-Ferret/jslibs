TARGET     = jsfastcgi.so
SRC        = $(wildcard *.cpp)
INCLUDES   = -I../../libs/fastcgi/src/include
STATICLIBS = -L../../libs/fastcgi/$(INT_DIR)libfcgi/.libs/libfcgi.a
SHAREDLIBS =
DEPENDS    = ../../libs/fastcgi/Makefile

include ../rules.mk
