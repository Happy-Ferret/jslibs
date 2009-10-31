TARGET=jshost
SRC=$(wildcard *.cpp)
INCLUDES=
STATICLIBS=../host/$(INT_DIR)host.a ../jslang/$(INT_DIR)jslang.a ../../libs/nedmalloc/$(INT_DIR)libnedmalloc.a -lm
SHAREDLIBS=-ldl -lpthread
DEPENDS=../host/Makefile ../jslang/Makefile ../tools/Makefile ../../libs/nedmalloc/Makefile

%.js.xdr.cres: %.js.xdr
	../tools/fileToCRes $? $@

%.js.xdr: %.js
	LD_LIBRARY_PATH=../../libs/js/$(INT_DIR)dist/bin:$LD_LIBRARY_PATH ../tools/jsScriptToXDR $? $@ true bootstrap

all:: $(DEPENDS) embeddedBootstrapScript.js.xdr.cres;

include ../rules.mk

