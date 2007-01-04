OBJS      = blob.o database.o error.o result.o main.o
LIBNAME   = jssqlite
CFLAGS    = -Wall
 
all: $(LIBNAME).so
 
$(LIBNAME).so: $(OBJS)
	gcc -shared $(CFLAGS) $(OBJS) -lc -Wl,-soname,$(LIBNAME).so -o $(LIBNAME).so

.cpp.o:
        gcc -fPIC -DPIC $(CFLAGS) -c -o $*.o $<
 
clean:
        rm -f *.o *.a *.so

