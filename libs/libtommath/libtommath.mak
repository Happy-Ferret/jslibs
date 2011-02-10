# This makefile modify the behavior of makefile.msvc because this version of makefile.msvc works only with MSVC 6.00 SP5

include makefile.msvc

# doc. http://msdn.microsoft.com/en-us/library/9s7c9wdw(v=vs.80).aspx
CFLAGS = 
!IF "$(debug)"=="1"
CFLAGS = $(CFLAGS) /MDd /Od /GF /Gm /RTC1 /arch:SSE /GR- /W4 /ZI
!ELSE
CFLAGS = $(CFLAGS) /MD /MP /W0 /Ox /Ob2 /Oi /Ot /Oy /GL /GF /FD /GS- /Gy /arch:SSE /GR- /Zi
!ENDIF


CFLAGS = $(CFLAGS) /Fd"..\Debug\vc90.pdb"

CFLAGS = $(CFLAGS) /I. /DWIN32 /DLTM_DESC /DXMALLOC=jl_malloc_fct /DXCALLOC=jl_calloc_fct /DXREALLOC=jl_realloc_fct /DXFREE=jl_free_fct
LIBNAME = $(dest)

.c.obj:
	cl /nologo $(CFLAGS) /Fo$@ /c $<

$(LIBNAME): $(OBJECTS)
	-mkdir "$(@D)"
	lib /nologo /LTCG /out:$@ $(OBJECTS)

build: $(LIBNAME)

clean:
	-del "$(LIBNAME)"
	-del /s *.obj

rebuild: clean build
