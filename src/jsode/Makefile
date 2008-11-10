TARGET     = jsode.so
SRC        = $(wildcard *.cpp)
INCLUDES   = -I../../libs/ode/src/include
STATICLIBS = -L../../libs/ode/$(BUILD) -lfast -lGIMPACT -lode
SHAREDLIBS =
DEPENDS    = ../../libs/ode/Makefile

include ../rules.mk

