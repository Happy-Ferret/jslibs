.DEFUALT: ;

.PHONY: $(MAKECMDGOALS)
$(MAKECMDGOALS): ./src/Makefile
	$(MAKE) -C src $@
	
./src/Makefile:
	cd src && ./configure --disable-debug --enable-strip --enable-nspr-threads --enable-optimize=-O3

