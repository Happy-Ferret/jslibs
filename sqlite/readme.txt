current Version: 3.3.8

http://www.sqlite.org/download.html
	Source Code
		sqlite-source-3_3_8.zip : for windows
		sqlite-3.3.8.tar.gz : for linux

.
..
src
readme.txt


[TBD] try to import from: 
  http://svn.python.org/projects/external/sqlite-source-3.3.4/
  (http://websvn.kde.org/branches/stable/extragear/graphics/digikam/sqlite/)


Linux compilation:

	./configure --disable-tcl --disable-dynamic --disable-shared --enable-static
	make clean all
	ls .libs/libsqlite.a
