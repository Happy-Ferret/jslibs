# This makefile modify the behavior of makefile.msvc because this version of makefile.msvc works only with MSVC 6.00 SP5

include makefile.msvc

CFLAGS = /I. /Ox /DWIN32 /DLTM_DESC /W3
LIBNAME = $(dest)

.c.obj:
    cl /nologo $(CFLAGS) /Fo$@ /c $<

$(LIBNAME): $(OBJECTS)
	lib /nologo /out:$(LIBNAME) $(OBJECTS)
	del /s *.obj

build: $(LIBNAME)

clean:
	del "$(LIBNAME)"
	del /s *.obj

rebuild: clean build
