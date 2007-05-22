CC= g++
AR= ar
RM= rm
LD= ld

SMINCLUDE= -I../js/src/Linux_All_OPT.OBJ -I../js/src
SMLIB= -Wl,-Bdynamic -L../js/src/Linux_All_OPT.OBJ -ljs

#ifeq ($(BUILD),dbg)
	CFLAGS= -Wall -g3
#else
	CFLAGS= -Wall -O3 -s -funroll-loops
#endif

OBJ= $(SRC:.cpp=.o)

.cpp.o:
	$(CC) -c $(CFLAGS) $(SMINCLUDE) $(INCLUDES) -o $*.o $<
	# -fpic needed ???

%.a: $(OBJ)
	$(AR) rcs $@ $^

%.so: $(OBJ)
	$(CC) $(CFLAGS) -o $@ -shared -Wl,-soname,$@ $^ -shared-libgcc -Wl,-Bstatic $(STATICLIBS) -Wl,-Bdynamic $(SHAREDLIBS) $(SMLIB)

%.bin: $(OBJ)
	$(CC) $(CFLAGS) -o $(basename $@) $^ -shared-libgcc -Wl,-Bstatic $(STATICLIBS) -Wl,-Bdynamic $(SHAREDLIBS) $(SMLIB)

all: $(TARGET)

clean:
	$(RM) -f *.o $(TARGET) $(basename $(TARGET))




#
# make manual: http://www.gnu.org/software/make/manual/make.html#Automatic-Variables
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

