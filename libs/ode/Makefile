include ../subdirs.mk

THISDIR=$(PWD)

TARGET_FILES = ./src/ode/src/libfast.a ./src/ode/src/libOPCODE.a ./src/ode/src/libode.a 

$(INT_DIR)Makefile:
	mkdir -p $(INT_DIR)
	cd $(INT_DIR) && ../src/configure --disable-shared --enable-static --disable-dependency-tracking --enable-release --disable-demos --with-trimesh=gimpact --with-drawstuff=none
	#--prefix=$(THISDIR)/$(INT_DIR)

all clean distclean:: $(INT_DIR)Makefile
	$(MAKE) -C $(INT_DIR)ode $@

