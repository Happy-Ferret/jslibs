LIBNAME = jssqlite
OBJS    = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
CFLAGS  = -g -Wall
LIBS    =  -L../sqlite/libs -lsqlite
 
.cpp.o:
	gcc -I../js/src -I../js/src/Linux_All_DBG.OBJ -fpic $(CFLAGS) -c -o $*.o $<

$(LIBNAME).so: $(OBJS)
	gcc -g -shared $(OBJS) -lc $(LIBS) -Wl,-soname,$(LIBNAME).so -o $(LIBNAME).so

all: $(LIBNAME).so
 
clean:
	rm -f *.o *.a *.so
