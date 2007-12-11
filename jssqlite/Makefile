TARGET     = jssqlite.so
SRC        = $(wildcard *.cpp)
INCLUDES   = -I../sqlite/src
STATICLIBS = ../sqlite/libsqlite3.a -ldl -lpthread
SHAREDLIBS =
DEPENDS    = ../sqlite/Makefile

include ../common/rules.mk
