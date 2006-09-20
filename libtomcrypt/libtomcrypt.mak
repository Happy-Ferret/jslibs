include makefile.msvc

CFLAGS = 

.c.obj:
    cl /Isrc/headers/ /Itestprof/ /Ox /DWIN32 /DLTC_SOURCE /W3 /Fo$@ /c $<

libonly: $(OBJECTS)
	lib /out:tomcrypt.lib $(OBJECTS)
