LIBNAME = jssqlite
OBJS    = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
CFLAGS  = -g -Wall
LIBS    =  -L../sqlite/libs -lsqlite
 
.cpp.o:
	g++ -I../js/src -I../js/src/Linux_All_DBG.OBJ -fpic $(CFLAGS) -c -o $*.o $<

$(LIBNAME).so: $(OBJS)
	g++ -shared $(OBJS) -Wl,-soname,$(LIBNAME).so -o $(LIBNAME).so -lc $(LIBS)

all: $(LIBNAME).so
 
clean:
	rm -f *.o *.a *.so
