TARGET=jshost
SRC=$(wildcard *.cpp)
INCLUDES=
STATICLIBS=../host/$(INT_DIR)host.a ../jslang/$(INT_DIR)jslang.a -lm
SHAREDLIBS=-ldl
DEPENDS=../host/Makefile ../jslang/Makefile

include ../rules.mk
