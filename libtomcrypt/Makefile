.DEFUALT: ;

.PHONY: $(MAKECMDGOALS)
$(MAKECMDGOALS):
	CFLAGS="-DLTM_DESC -I../../libtommath/src" $(MAKE) -C src $(subst all,default,$@)

