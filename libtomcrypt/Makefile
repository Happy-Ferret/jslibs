include ../common/subdirs.mk

TARGET_FILES = ./src/libtomcrypt.a

all:: 
	CFLAGS="-DLTM_DESC -I../../libtommath/src" $(MAKE) -C src default
clean::
	-patch -N -p0 -i libtomcrypt.patch #avoid conflict between src/makefile's clean goal and src/.svn/ content
	$(MAKE) -C src clean
