# This makefile modify the behavior of makefile.msvc because this version of makefile.msvc works only with MSVC 6.00 SP5

include makefile.msvc

CFLAGS = /I. /W3 /Ox /DWIN32 /DLTM_DESC /DXMALLOC=ltc_malloc /DXCALLOC=ltc_calloc /DXREALLOC=ltc_realloc /DXFREE=ltc_free
LIBNAME = $(dest)

.c.obj:
	cl /nologo $(CFLAGS) /Fo$@ /c $<

$(LIBNAME): $(OBJECTS)
	-mkdir "$(@D)"
	lib /nologo /out:$@ $(OBJECTS)

build: $(LIBNAME)

clean:
	-del "$(LIBNAME)"
	-del /s *.obj

rebuild: clean build
