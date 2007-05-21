LIBNAME    = jsnspr
OBJS       = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
INCLUDES   = -I../nspr/nsprpub/dist/include/nspr
STATICLIBS = -L../nspr/nsprpub/dist/lib -lnspr4
SHAREDLIBS =

include ../common/rules.mk
