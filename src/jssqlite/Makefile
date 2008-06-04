TARGET     = jssqlite.so
SRC        = $(wildcard *.cpp)
INCLUDES   = -I../sqlite/src
STATICLIBS = ../sqlite/src/libsqlite3.a
SHAREDLIBS = -ldl -lpthread
DEPENDS    = ../sqlite/Makefile

include ../common/rules.mk
