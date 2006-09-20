libtomcrypt source files:
	http://libtomcrypt.com/download.html

default version:
	v1.14

directories structure
	.
	..
	README.TXT
	libtomcrypt.mak
	libtomcrypt.vcproj
	...
	src (directory from the libtomcrypt archive)
	makefile.msvc (file from the libtomcrypt archive)
	...

another way to build

	create build.cmd:
		call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /XP32 /RETAIL
		call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86
		nmake /f libtomcrypt.mak libonly

	create libtomcrypt.mak:
		include makefile.msvc

		CFLAGS = 

		.c.obj:
			cl /Isrc/headers/ /Itestprof/ /Ox /DWIN32 /DLTC_SOURCE /W3 /Fo$@ /c $<

		libonly: $(OBJECTS)
			lib /out:tomcrypt.lib $(OBJECTS)

	...