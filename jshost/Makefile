OBJS      = jshost.o
BINNAME   = jshost
CFLAGS    = -Wall
 
all: $(BINNAME)
 
$(BINNAME): $(OBJS)
	gcc $(CFLAGS) $(OBJS) -lc -o $(BINNAME)

.cpp.o:
	gcc -I../js/src -I../js/src/Linux_All_DBG.OBJ $(CFLAGS) -c -o $*.o $<
 
clean:
	rm -f *.o *.a *.so

