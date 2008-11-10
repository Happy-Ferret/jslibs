!IF "$(BUILD)" == "Debug"
MFLAGS=-MDd
!ENDIF

!IF "$(BUILD)" == "Release"
MFLAGS=-MD
!ENDIF

clean::
	cd .\src && $(MAKE) -f Makefile.msvc NO_NLS=1 MFLAGS=$(MFLAGS) distclean
	-del .\src\config.h
	-cd "$(OUTDIR)" && del iconv.lib
	-cd "$(OUTDIR)" && del charset.lib
	
all::
	copy /Y .\src\config.h.msvc .\src\config.h
	cd .\src && $(MAKE) -f Makefile.msvc NO_NLS=1 MFLAGS=$(MFLAGS)
	-mkdir "$(OUTDIR)"
	copy /Y .\src\lib\iconv.lib "$(OUTDIR)"
	copy /Y .\src\libcharset\lib\charset.lib "$(OUTDIR)"
