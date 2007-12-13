.DEFUALT: ;

.PHONY: $(MAKECMDGOALS)
$(MAKECMDGOALS): ./src/Makefile
	$(MAKE) -C src $@
	
./src/Makefile:
	cd src && ./configure --enable-static --disable-shared

