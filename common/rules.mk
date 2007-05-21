#ifeq ($(BUILD),opt)
CFLAGS = -O3 -s -funroll-loops
#else
CFLAGS = -g3
#endif

.cpp.o:
	g++ -Wall -c -fpic $(CFLAGS) -I../js/src -I../js/src/Linux_All_DBG.OBJ $(INCLUDES) -o $*.o $<

$(LIBNAME).so: $(OBJS)
	g++ -Wall -shared $(OBJS) -Wl,-soname,$(LIBNAME).so -o $(LIBNAME).so -shared-libgcc -Wl,-Bstatic $(STATICLIBS) -Wl,-Bdynamic -L../js/src/Linux_All_DBG.OBJ -ljs $(SHAREDLIBS)

all: $(LIBNAME).so

clean:
	rm -f *.o *.a *.so

# 

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


#Linking with a mix of shared and static libraries
# http://linux4u.jinr.ru/usoft/WWW/www_debian.org/Documentation/elf/node18.html
# gcc -o main main.o -Wl,-Bstatic -lfoo -Wl,-Bdynamic -lbar

# __gxx_personality_v0 ...error: use g++ instead of gcc
# 

#ERROR: error while loading shared libraries: libstdc++.so.6: cannot open shared object file: No such file or directory
# If you install libstdc++.so.6 in a non-standard location, you need to tell the dynamic linker where to find it.  This can be done by setting
# the environment variable LD_LIBRARY_PATH to the directory where libstdc++.so.6 can be found.  Or by using -Wl,-rpath,DIR when you
# link.  Or, on a GNU/Linux system, by adding the directory to /etc/ld.so.conf and friends and running ldconfig.
