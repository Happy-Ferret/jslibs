# This makefile modify the behavior of makefile.msvc because this version of makefile.msvc works only with MSVC 6.00 SP5

include makefile.msvc

CFLAGS = 

.c.obj:
    cl /I. /Ox /DWIN32 /DLTM_DESC /W3 /Fo$@ /c $<

libonly: $(OBJECTS)
	lib /out:tommath.lib $(OBJECTS)
