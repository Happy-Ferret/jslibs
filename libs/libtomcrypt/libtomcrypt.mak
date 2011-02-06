# This makefile modify the behavior of makefile.msvc because this version of makefile.msvc works only with MSVC 6.00 SP5

include makefile.msvc

# doc. http://msdn.microsoft.com/en-us/library/9s7c9wdw(v=vs.80).aspx
CFLAGS = /MP /nologo /W0 /Ox /Ob2 /Oi /Ot /Oy /GL /GF /FD /MD /GS- /Gy /arch:SSE /GR- /Zi
CFLAGS = $(CFLAGS) /Isrc/headers/ /I../../libtommath/src/ /DWIN32 /DLTC_SOURCE /DLTM_DESC /DMKAT /DNO_FILE /DXMALLOC=jl_malloc_fct /DXCALLOC=jl_calloc_fct /DXREALLOC=jl_realloc_fct /DXFREE=jl_free_fct

### (TBD)
### OPT FLAGS: /Ox /Ob2 /Ot /GL
### DBG FLAGS: /Od /Yd /Zi /ZI
### DEFAULT FLAGS: /Isrc/headers/ /Itestprof/ /Ox /DWIN32 /DLTC_SOURCE /W3 /Fo$@ $(CF)

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
