call ..\..\..\envbuild.cmd

cd ../src
sh ./configure --enable-shared=no --enable-static=yes --prefix=$(_INSTALL_DIR) GLIB_CFLAGS=" " GLIB_LIBS=" " LIBRSVG_CFLAGS=" " LIBRSVG_LIBS=" "

pause