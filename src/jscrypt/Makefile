TARGET     = jscrypt.so
SRC        = $(wildcard *.cpp)
INCLUDES   = -I../../libs/libtommath/src -I../../libs/libtomcrypt/src/src/headers
#STATICLIBS =  -L../../libs/libtommath/src -ltommath -L../../libs/libtomcrypt/src -ltomcrypt
STATICLIBS = ../../libs/libtomcrypt/src/libtomcrypt.a ../../libs/libtommath/src/libtommath.a
SHAREDLIBS =
DEFINES    = -DLTM_DESC
DEPENDS    = ../../libs/libtommath/Makefile ../../libs/libtomcrypt/Makefile

include ../common/rules.mk

#
#note:
# library containing the definition of a function should appear after any source files or object files which use it.
#
#
