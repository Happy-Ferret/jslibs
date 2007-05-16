BINNAME = jshost
 
$(BINNAME):
	g++ -Wall -O3 -o $(BINNAME) jshost.cpp -I../js/src -I../js/src/Linux_All_DBG.OBJ ../moduleManager/moduleManager.a -L../js/src/Linux_All_DBG.OBJ -ljs -ldl

all: $(BINNAME)
 
clean:
	rm -f *.o *.a *.so $(BINNAME)

