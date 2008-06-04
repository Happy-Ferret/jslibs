Linux compilation:
-----------------

__declspec(dllexport)
	http://gcc.gnu.org/wiki/Visibility
	
makefile
	http://www.yiluda.net/manual/linux/rute/node26.html

Program Library HOWTO
	http://www.dwheeler.com/program-library/Program-Library-HOWTO/index.html

misc:
----


http://www.linuxdoc.org/HOWTO/Program-Library-HOWTO/index.html

...

To dumb down your shared library so that it resembles a Windows DLL,
you should use the -Bsymbolic linker option, which you can pass down to
the linker through the gcc front end using

    -Wl,-Bsymbolic 