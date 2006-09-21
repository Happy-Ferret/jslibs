# This makefile modify the behavior of makefile.msvc because this version of makefile.msvc works only with MSVC 6.00 SP5

include makefile.msvc

CFLAGS = 

.c.obj:
    cl /Isrc/headers/ /I../libtommath/ /Ox /DWIN32 /DLTC_SOURCE /DLTM_DESC /W3 /Fo$@ /c $<

libonly: $(OBJECTS)
	lib /out:tomcrypt.lib $(OBJECTS)

clean:
	del tomcrypt.lib
	del /s *.obj

rebuild: clean libonly