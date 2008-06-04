TARGET     = jsio.so
SRC        = $(wildcard *.cpp)
INCLUDES   = -I../nspr/src/dist/include/nspr -I../nspr/src/dist/include/nspr/private
#STATICLIBS = -L../nspr/src/dist/lib -lnspr4
SHAREDLIBS = -L../nspr/src/dist/lib -lnspr4
DEPENDS    = ../nspr/Makefile

include ../common/rules.mk
