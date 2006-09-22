# This makefile modify the behavior of makefile.msvc because this version of makefile.msvc works only with MSVC 6.00 SP5

include makefile.msvc

CFLAGS = /Isrc/headers/ /I../libtommath/ /Ox /DWIN32 /DLTC_SOURCE /DLTM_DESC /W3
LIBNAME = tomcrypt.lib

.c.obj:
    cl /nologo $(CFLAGS) /Fo$@ /c $<

$(LIBNAME): $(OBJECTS)
	lib /nologo /out:$(LIBNAME) $(OBJECTS)

build: $(LIBNAME)

clean:
	del $(LIBNAME)
	del /s *.obj

rebuild: clean build