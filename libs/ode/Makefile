include ../subdirs.mk

TARGET_FILES = ./src/ode/src/libfast.a ./src/ode/src/libOPCODE.a ./src/ode/src/libode.a 

./src/Makefile:
	cd ./src && ./configure --disable-shared --enable-static --disable-dependency-tracking --enable-release --disable-demos --with-trimesh=opcode
	
all clean distclean:: ./src/Makefile
	$(MAKE) -C src/ode $@

