LIBNAME = moduleManager
OBJS    = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
CFLAGS  = -g -Wall
LIBS    =

.cpp.o:
	g++ -c -I../js/src -I../js/src/Linux_All_DBG.OBJ $(CFLAGS) -o $*.o $<

$(LIBNAME).a: $(OBJS)
	ar rcs $(LIBNAME).a $(OBJS)

all: $(LIBNAME).a
 
clean:
	rm -f *.o *.a *.so

# http://tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html
# debugging information: -g
# generate warnings -Wall
# Using -fpic option usually generates smaller and faster code, but will have platform-dependent limitations ...
# Linking against static library: gcc -static main.c -L. -lmean -o statically_linked
# Linking against shared library: gcc main.c -o dynamically_linked -L. -lmean
