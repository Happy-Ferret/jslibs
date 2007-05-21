LIBNAME    = jssqlite
OBJS       = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
INCLUDES   = -I../sqlite/sqlite-3.3.17
STATICLIBS = -L../sqlite/sqlite-3.3.17/.libs -lsqlite3
SHAREDLIBS =

include ../common/rules.mk
