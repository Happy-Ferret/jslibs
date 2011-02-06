!IF "$(BUILD)" == "Debug"
MFLAGS=-MDd
!ENDIF

!IF "$(BUILD)" == "Release"
MFLAGS=-MD
!ENDIF

#  -Ox -Ob2 -Ot -GL

# doc. http://msdn.microsoft.com/en-us/library/9s7c9wdw(v=vs.80).aspx
MFLAGS = $(MFLAGS) /MP /nologo /Ox /Ob2 /Oi /Ot /Oy /GL /GF /FD /GS- /Gy /arch:SSE /GR- /Zi


clean::
	cd .\src && $(MAKE) -f Makefile.msvc NO_NLS=1 MFLAGS="$(MFLAGS)" distclean
	-del .\src\config.h
	-cd "$(OUTDIR)" && del iconv.lib
	-cd "$(OUTDIR)" && del charset.lib
	
all::
	copy /Y .\src\config.h.msvc .\src\config.h
	cd .\src && $(MAKE) -f Makefile.msvc NO_NLS=1 MFLAGS="$(MFLAGS)"
	-mkdir "$(OUTDIR)"
	copy /Y .\src\lib\iconv.lib "$(OUTDIR)"
	copy /Y .\src\libcharset\lib\charset.lib "$(OUTDIR)"
