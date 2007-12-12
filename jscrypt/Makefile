TARGET     = jscrypt.so
SRC        = $(wildcard *.cpp)
INCLUDES   = -I../libtommath/src -I../libtomcrypt/src/src/headers
STATICLIBS =  -L../libtommath/src -ltommath -L../libtomcrypt/src -ltomcrypt
SHAREDLIBS =
DEPENDS    = ../libtommath/Makefile ../libtomcrypt/Makefile

include ../common/rules.mk
