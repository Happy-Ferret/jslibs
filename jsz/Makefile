LIBNAME    = jsz
OBJS       = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
INCLUDES   = -I../zlib/src
STATICLIBS = -L../zlib/src -lz
SHAREDLIBS =

include ../common/rules.mk
