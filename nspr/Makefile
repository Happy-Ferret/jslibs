.PHONY: all clean distclean install

all: ./src/Makefile
	cd src && make $(MAKECMDGOALS)

clean distclean install:
	cd src && $(MAKE) $(MAKECMDGOALS)

./src/Makefile:
	cd src && ./configure --disable-debug --enable-strip --enable-nspr-threads --enable-optimize=-O3

