# This makefile modify the behavior of makefile.msvc because this version of makefile.msvc works only with MSVC 6.00 SP5

include makefile.msvc

CFLAGS = /Isrc/headers/ /I../../libtommath/src/ /DWIN32 /DLTC_SOURCE /DLTM_DESC /DNO_FILE /W3 /Ox
### DEBUG VERSION: /Od /Yd /Zi /ZI
### DEFAULT FLAGS: /Isrc/headers/ /Itestprof/ /Ox /DWIN32 /DLTC_SOURCE /W3 /Fo$@ $(CF)

LIBNAME = $(dest)

.c.obj:
	cl /nologo $(CFLAGS) /Fo$@ /c $<

$(LIBNAME): $(OBJECTS)
	mkdir "$(@D)"
	lib /nologo /out:$@ $(OBJECTS)

build: $(LIBNAME)

clean:
	-del "$(LIBNAME)"
	-del /s *.obj

rebuild: clean build
