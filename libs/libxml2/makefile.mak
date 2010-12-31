.\src\win32\config.msvc:
	cd .\src\win32 && cscript configure.js cruntime="$(MFLAGS) $(CFLAGS)" \
    trio=no \
    threads=no \
    ftp=no \
    http=no \
    html=no \
    c14n=no \
    catalog=yes \
    docb=no \
    xpath=yes \
    xptr=no \
    xinclude=no \
    iconv=no \
    iso8859x=no \
    zlib=yes \
    regexps=yes \
    modules=no \
    tree=yes \
    reader=no \
    writer=no \
    walker=no \
    pattern=yes \
    push=yes \
    valid=yes \
    sax1=yes \
    legacy=no \
    output=yes \
    schemas=yes \
    schematron=no \
    python=no \
    compiler=msvc \
    static=yes \
#!IF "$(BUILD)" == "DBG"
#	debug=yes \
#!ENDIF
    lib=..\..\..\zlib\src\lib \
    include=..\..\..\zlib\src

configure: .\src\win32\config.msvc

clean:
	-if exist .\src\win32\Makefile cd .\src\win32 && $(MAKE) -f Makefile.msvc distclean
	-del "$(TARGET_DIR)\$(TARGET_FILE)"

all: configure
	cd .\src\win32 && $(MAKE) -f Makefile.msvc libxmla
	-mkdir "$(TARGET_DIR)"
	copy /Y .\src\win32\bin.msvc\libxml2_a.lib "$(TARGET_DIR)\$(TARGET_FILE)"
