TARGET     = jsio.so
SRC        = $(wildcard *.cpp)
INCLUDES   = -I../../libs/nspr/$(INT_DIR)dist/include/nspr -I../../libs/nspr/$(INT_DIR)dist/include/nspr/private
#STATICLIBS = -L../../libs/nspr/src/dist/lib -lnspr4
SHAREDLIBS = -L../../libs/nspr/$(INT_DIR)dist/lib -lnspr4
DEPENDS    = ../../libs/nspr/Makefile

include ../rules.mk
