TARGET     = jscrypt.so
SRC        = $(wildcard *.cpp)
INCLUDES   = -I../libtommath/src -I../libtomcrypt/src/src/headers
#STATICLIBS =  -L../libtommath/src -ltommath -L../libtomcrypt/src -ltomcrypt
STATICLIBS = ../libtomcrypt/src/libtomcrypt.a ../libtommath/src/libtommath.a
SHAREDLIBS =
DEFINES    = -DLTM_DESC
DEPENDS    = ../libtommath/Makefile ../libtomcrypt/Makefile

include ../common/rules.mk

#
#note:
# library containing the definition of a function should appear after any source files or object files which use it.
#
#
