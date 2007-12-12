TARGET     = jssqlite.so
SRC        = $(wildcard *.cpp)
INCLUDES   = -I../sqlite/src
STATICLIBS =  -ldl -lpthread -L../sqlite -lsqlite3
SHAREDLIBS =
DEPENDS=../sqlite/Makefile

include ../common/rules.mk
