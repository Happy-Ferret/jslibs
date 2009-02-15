TARGET     = jssqlite.so
SRC        = $(wildcard *.cpp)
INCLUDES   = -I../../libs/sqlite/src
STATICLIBS = ../../libs/sqlite/$(INT_DIR)libsqlite3.a
SHAREDLIBS = -ldl -lpthread
DEPENDS    = ../../libs/sqlite/Makefile

include ../rules.mk
