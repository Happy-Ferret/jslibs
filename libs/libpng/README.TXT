svn:externals src http://libset.googlecode.com/svn/trunk/libpng/

Download:
	http://www.libpng.org/pub/png/libpng.html
	http://prdownloads.sourceforge.net/libpng/lpng1212.zip?download
	
Install src
	unzip source files into src/ directory (src/ has to be create)

Configuration:
	In Configuration Properties > General > Inherited Project Property Sheets,
	let jslibs\libpng\src\projects\visualc???\???.vcproj file inherit from jslibs\libpng\libpng.vsprops ( or ..\..\..\libpng.vsprops ? )
	
	
Link with (OPT):
..\..\libs\libtomcrypt\Release\tomcrypt.lib ..\..\libs\libtommath\Release\tommath.lib advapi32.lib
Link with (DBG):
..\..\libs\libtomcrypt\Debug\tomcrypt.lib ..\..\libs\libtommath\Debug\tommath.lib advapi32.lib
