TARGET     = jssqlite.so
SRC        = $(wildcard *.cpp)
INCLUDES   = -I../sqlite/src
STATICLIBS =  -ldl -lpthread -L../src/sqlite/libsqlite3.a
SHAREDLIBS =
DEPENDS    = ../sqlite/Makefile

include ../common/rules.mk
