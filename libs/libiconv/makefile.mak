
# doc. http://msdn.microsoft.com/en-us/library/9s7c9wdw(v=vs.80).aspx
MFLAGS =
!IF "$(BUILD)" == "Debug"
MFLAGS = $(MFLAGS) /MDd /Od /GF /Gm /RTC1 /RTCc /arch:SSE /GR- /W3 /ZI /TP /Yd
!ENDIF
!IF "$(BUILD)" == "Release"
MFLAGS = $(MFLAGS) /MD /MP /Ox /Ob2 /Oi /Ot /Oy /GL /GF /FD /GS- /Gy /arch:SSE /GR- /Zi
!ENDIF

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
