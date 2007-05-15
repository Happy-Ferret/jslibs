BINNAME = jshost
CFLAGS  = -g -Wall
 
$(BINNAME):
	g++ $(CFLAGS) -o $(BINNAME) jshost.cpp -I../js/src -I../js/src/Linux_All_DBG.OBJ ../moduleManager/moduleManager.a -L../js/src/Linux_All_DBG.OBJ -ljs -ldl

all: $(BINNAME)
 
clean:
	rm -f *.o *.a *.so $(BINNAME)

# __gxx_personality_v0 ...error: use g++ instead of gcc
# 

#ERROR: error while loading shared libraries: libstdc++.so.6: cannot open shared object file: No such file or directory
# If you install libstdc++.so.6 in a non-standard location, you need to tell the dynamic linker where to find it.  This can be done by setting
# the environment variable LD_LIBRARY_PATH to the directory where libstdc++.so.6 can be found.  Or by using -Wl,-rpath,DIR when you
# link.  Or, on a GNU/Linux system, by adding the directory to /etc/ld.so.conf and friends and running ldconfig.
