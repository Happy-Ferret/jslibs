BUILD ?= opt
BITS ?= 32
INT_DIR = $(shell uname)_$(BUILD)_$(BITS)/
JSDIR=../../libs/js/$(INT_DIR)dist/

all: jsScriptToXDR fileToCRes;

clean:
	-rm fileToCRes jsScriptToXDR

fileToCRes: fileToCRes.cpp
	g++ $? -o $@ -O3

jsScriptToXDR: jsScriptToXDR.cpp
	g++ $? -o $@ -O3 -I$(JSDIR)include/js -L$(JSDIR)lib -lmozjs
