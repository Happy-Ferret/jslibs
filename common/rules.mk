#ifeq ($(BUILD),opt)
#endif

.cpp.o:
	g++ -Wall -c -I../js/src -I../js/src/Linux_All_DBG.OBJ -fpic $(CFLAGS) -o $*.o $<

$(LIBNAME).so: $(OBJS)
	g++ -Wall -shared-libgcc -shared $(OBJS) -Wl,-soname,$(LIBNAME).so -o $(LIBNAME).so ../js/src/Linux_All_DBG.OBJ/libjs.so $(LIBS)

all: $(LIBNAME).so

clean:
	rm -f *.o *.a *.so



# http://tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html
# debugging information: -g
# generate warnings -Wall
# Using -fpic option usually generates smaller and faster code, but will have platform-dependent limitations ...
# Linking against static library: gcc -static main.c -L. -lmean -o statically_linked
# Linking against shared library: gcc main.c -o dynamically_linked -L. -lmean

#-shared-libgcc
#-static-libgcc
#	On systems that provide libgcc as a shared library, these options force the use of either the shared or static version respectively.  If no shared
#	version of libgcc was built when the compiler was configured, these options have no effect.
