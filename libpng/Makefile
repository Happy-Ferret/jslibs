.DEFUALT: ;

.PHONY: $(MAKECMDGOALS)
$(MAKECMDGOALS): ./src/Makefile
	LDFLAGS="-L../../zlib/src" INCLUDES="-I../../zlib/src" $(MAKE) -C src $@
	
./src/Makefile:
	LDFLAGS="-L../../zlib/src" INCLUDES="-I../../zlib/src" cd src && ./configure --disable-shared

