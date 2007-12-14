.DEFUALT: ;
.PHONY: $(MAKECMDGOALS)
$(MAKECMDGOALS): 
	CFLAGS="-DLTM_DESC -I../../libtommath/src" $(MAKE) -C src $(@:all=default)
clean:
	-patch -N -p0 -i libtomcrypt.patch
	$(MAKE) -C src clean
