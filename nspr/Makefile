.PHONY: all clean install

all: ./src/Makefile
	cd src && make $(MAKECMDGOALS)

clean install:
	cd src && $(MAKE) $(MAKECMDGOALS)

./src/Makefile:
	cd src && ./configure --disable-debug --enable-strip --enable-nspr-threads --enable-optimize=-O3

